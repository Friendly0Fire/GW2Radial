#include <Core.h>
#include <Direct3D11Loader.h>
#include <imgui.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#include <Input.h>
#include <ConfigurationFile.h>
#include <Wheel.h>
#include <Mount.h>
#include <SettingsMenu.h>
#include <Utility.h>
#include <imgui_internal.h>
#include <Novelty.h>
#include <shellapi.h>
#include <UpdateCheck.h>
#include <ImGuiPopup.h>
#include <Marker.h>
#include <MiscTab.h>
#include <MumbleLink.h>
#include <ShaderManager.h>
#include <CustomWheel.h>
#include <GFXSettings.h>
#include <Log.h>
#include <Version.h>
#include <MiscTab.h>

namespace GW2Radial
{

class RadialMiscTab : public ::MiscTab
{
	bool reloadOnFocus_ = false;
public:
	void AdditionalGUI() override
	{
		ImGuiTitle("Toggle Menu Visibility");

		for (auto& wheel : Core::i().wheels()) {
			ImGuiConfigurationWrapper(ImGui::Checkbox, wheel->visibleInMenuOption());
		}

		ImGuiTitle("Custom Wheel Tools");

		ImGui::Checkbox("Reload custom wheels on focus", &reloadOnFocus_);

		if (ImGui::Button("Reload custom wheels"))
			Core::i().ForceReloadWheels();
	}

	bool reloadOnFocus() const { return reloadOnFocus_; }
};

void Core::InnerFrequentUpdate()
{
	const auto& mumble = MumbleLink::i();

	uint map = mumble.mapId();
	if(map != mapId_)
	{
	    for (auto& wheel : wheels_)
		    wheel->OnMapChange(mapId_, map);

	    mapId_ = map;
	}

	if(characterName_ != mumble.characterName())
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

	bgTex_ = std::make_shared<Texture2D>(std::move(CreateTextureFromResource(device_.Get(), Core::i().dllModule(), IDR_BG)));
	wheels_.emplace_back(Wheel::Create<Mount>(bgTex_, "mounts", "Mounts"));
	wheels_.emplace_back(Wheel::Create<Novelty>(bgTex_, "novelties", "Novelties"));
	wheels_.emplace_back(Wheel::Create<Marker>(bgTex_, "markers", "Markers"));
	wheels_.emplace_back(Wheel::Create<ObjectMarker>(bgTex_, "object_markers", "Object Markers"));
}

void Core::InnerInitPostImGui()
{
	customWheels_ = std::make_unique<CustomWheelsManager>(bgTex_, wheels_, fontDraw_);

	firstMessageShown_ = std::make_unique<ConfigurationOption<bool>>("", "first_message_shown_v1", "Core", false);
}

void Core::InnerInternalInit()
{
	ULONG_PTR contextToken;
	if (CoGetContextToken(&contextToken) == CO_E_NOTINITIALIZED) {
		HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		if (hr != S_FALSE && hr != RPC_E_CHANGED_MODE && FAILED(hr))
			CriticalMessageBox(L"Could not initialize COM library: error code 0x%X.", hr);
	}
}

void Core::InnerShutdown()
{
	CoUninitialize();
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
		ImGuiPopup("Welcome to GW2Radial!").Position({ 0.5f, 0.45f }).Size({ 0.35f, 0.2f }).Display([&](const ImVec2& windowSize)
			{
				ImGui::TextWrapped("Welcome to GW2Radial! This small addon shows a convenient, customizable radial menu overlay to select a mount or novelty on the fly for Guild Wars 2: Path of Fire. "
					"To begin, use the shortcut Shift+Alt+M to open the settings menu and take a moment to bind your keys. If you ever need further assistance, please visit "
					"this project's website at");

				ImGui::Spacing();
				ImGui::SetCursorPosX(windowSize.x * 0.1f);

				if (ImGui::Button("https://github.com/Friendly0Fire/GW2Radial", ImVec2(windowSize.x * 0.8f, ImGui::GetFontSize() * 1.3f)))
					ShellExecute(0, 0, L"https://github.com/Friendly0Fire/GW2Radial", 0, 0, SW_SHOW);
			}, [&]() { firstMessageShown_->value(true); });

	customWheels_->DrawOffscreen(context_.Get());

	if(forceReloadWheels_)
	{
	    forceReloadWheels_ = false;
	    customWheels_->MarkReload();
	}
}

}