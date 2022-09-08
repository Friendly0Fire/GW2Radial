#include <ConfigurationFile.h>
#include <Core.h>
#include <CustomWheel.h>
#include <Direct3D11Loader.h>
#include <GFXSettings.h>
#include <ImGuiPopup.h>
#include <Input.h>
#include <Log.h>
#include <MarkerWheel.h>
#include <MiscTab.h>
#include <MountWheel.h>
#include <MumbleLink.h>
#include <NoveltyWheel.h>
#include <SettingsMenu.h>
#include <ShaderManager.h>
#include <UpdateCheck.h>
#include <Utility.h>
#include <Version.h>
#include <Wheel.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <shellapi.h>

KeyCombo GetSettingsKeyCombo()
{
    return { GetScanCodeFromVirtualKey('M'), Modifier::SHIFT | Modifier::ALT };
}

namespace GW2Radial
{

class RadialMiscTab : public ::MiscTab
{
    bool reloadOnFocus_ = false;

public:
    void AdditionalGUI() override
    {
        ImGuiTitle("Toggle Menu Visibility");

        for (auto& wheel : Core::i().wheels())
        {
            ImGuiConfigurationWrapper(ImGui::Checkbox, wheel->visibleInMenuOption());
        }

        ImGuiTitle("Custom Wheel Tools");

        ImGui::Checkbox("Reload custom wheels on focus", &reloadOnFocus_);

        if (ImGui::Button("Reload custom wheels"))
            Core::i().ForceReloadWheels();
    }

    bool reloadOnFocus() const
    {
        return reloadOnFocus_;
    }
};

void Core::InnerFrequentUpdate()
{
    const auto& mumble = MumbleLink::i();

    uint        map    = mumble.mapId();
    if (map != mapId_)
    {
        for (auto& wheel : wheels_)
            wheel->OnMapChange(mapId_, map);

        mapId_ = map;
    }

    if (characterName_ != mumble.characterName())
    {
        for (auto& wheel : wheels_)
            wheel->OnCharacterChange(characterName_, mumble.characterName());

        characterName_ = mumble.characterName();
    }
}

void Core::InnerOnFocus()
{
    if (RadialMiscTab::i<RadialMiscTab>().reloadOnFocus())
        forceReloadWheels_ = true;
}

void Core::InnerOnFocusLost()
{
    for (auto& wheel : wheels_)
        if (wheel)
            wheel->OnFocusLost();
}

void Core::InnerInitPreImGui()
{
    RadialMiscTab::init<RadialMiscTab>();

    bgTex_ = std::make_shared<Texture2D>(CreateTextureFromResource(device_.Get(), i().dllModule(), IDR_BG));
    wheels_.push_back(std::make_unique<MountWheel>(bgTex_));
    wheels_.push_back(std::make_unique<NoveltyWheel>(bgTex_));
    wheels_.push_back(std::make_unique<MarkerWheel>(bgTex_));
    wheels_.push_back(std::make_unique<ObjectMarkerWheel>(bgTex_));

    vertexCB_ = ShaderManager::i().MakeConstantBuffer<VertexCB>();
}

void Core::InnerInitPostImGui()
{
    customWheels_      = std::make_unique<CustomWheelsManager>(bgTex_, wheels_, fontDraw_);

    firstMessageShown_ = std::make_unique<ConfigurationOption<bool>>("", "first_message_shown_v1", "Core", false);
}

void Core::InnerInternalInit()
{
    // COM concurrency model is annoying to deal with, can cause issues if enabled in a different way by another addon
    // However, with D3D11 in explicit single threaded mode, we CANNOT actually run background tasks, so instead
    // we only have one task and block the main thread. Stupid, but it isolates our own use/init of COM from others.
    comThread_ = std::make_unique<std::jthread>(
        [this](std::stop_token stopToken)
        {
            ULONG_PTR contextToken;
            if (CoGetContextToken(&contextToken) == CO_E_NOTINITIALIZED)
            {
                HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
                if (hr != S_FALSE && hr != RPC_E_CHANGED_MODE && FAILED(hr))
                    CriticalMessageBox(L"Could not initialize COM library: error code 0x%X.", hr);
            }

            while (!stopToken.stop_requested())
            {
                std::unique_lock lk(comTaskMutex_);
                using namespace std::chrono_literals;
                comNotify_.wait_for(lk, 100ms);

                if (comTask_)
                    (*comTask_)();
                comTask_ = std::nullopt;

                lk.unlock();
                comNotify_.notify_one();
            }

            CoUninitialize();
        });
}

void Core::InnerShutdown()
{
    comThread_.reset();
    wheels_.clear();
    customWheels_.reset();
    bgTex_.reset();
    vertexCB_.reset();
}

void Core::InnerUpdate()
{
    for (auto& wheel : wheels_)
        wheel->OnUpdate();
}

void Core::InnerDraw()
{
    for (auto& wheel : wheels_)
        wheel->Draw(context_.Get());

    customWheels_->Draw(context_.Get());

    if (!firstMessageShown_->value())
        ImGuiPopup("Welcome to GW2Radial!")
            .Position({ 0.5f, 0.45f })
            .Size({ 0.35f, 0.2f })
            .Display(
                [&](const ImVec2& windowSize)
                {
                    ImGui::TextWrapped(
                        "Welcome to GW2Radial! This small addon shows a convenient, customizable radial menu overlay to select a mount, novelty and more on the fly for Guild Wars "
                        "2. "
                        "To begin, use the shortcut Shift+Alt+M to open the settings menu and take a moment to bind your keys. If you ever need further assistance, please visit "
                        "this project's website at");

                    ImGui::Spacing();
                    ImGui::SetCursorPosX(windowSize.x * 0.1f);

                    if (ImGui::Button("https://github.com/Friendly0Fire/GW2Radial", ImVec2(windowSize.x * 0.8f, ImGui::GetFontSize() * 1.3f)))
                        ShellExecute(0, 0, L"https://github.com/Friendly0Fire/GW2Radial", 0, 0, SW_SHOW);
                },
                [&]() { firstMessageShown_->value(true); });

    customWheels_->DrawOffscreen(context_.Get());

    if (forceReloadWheels_)
    {
        forceReloadWheels_ = false;
        customWheels_->MarkReload();
    }
}

} // namespace GW2Radial