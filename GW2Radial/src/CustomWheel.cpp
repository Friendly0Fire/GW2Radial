#include <Core.h>
#include <CustomWheel.h>
#include <DirectXTK/DDSTextureLoader.h>
#include <DirectXTK/WICTextureLoader.h>
#include <FileSystem.h>
#include <ImGuiExtensions.h>
#include <ImGuiPopup.h>
#include <Wheel.h>
#include <backends/imgui_impl_dx11.h>
#include <filesystem>
#include <fstream>

namespace GW2Radial
{
float CalcText(ImFont* font, const std::wstring& text)
{
    const auto& txt = utf8_encode(text);

    auto        sz  = font->CalcTextSizeA(100.f, FLT_MAX, 0.f, txt.c_str());

    return sz.x;
}

RenderTarget MakeTextTexture(float fontSize)
{
    auto                 dev = Core::i().device();
    const auto           fmt = DXGI_FORMAT_R8G8B8A8_UNORM;

    RenderTarget         rt;
    D3D11_TEXTURE2D_DESC desc;
    desc.Format             = fmt;
    desc.Width              = 1024;
    desc.Height             = static_cast<uint>(fontSize);
    desc.BindFlags          = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.MipLevels          = 1;
    desc.ArraySize          = 1;
    desc.Usage              = D3D11_USAGE_DEFAULT;
    desc.CPUAccessFlags     = 0;
    desc.MiscFlags          = 0;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    GW2_CHECKED_HRESULT(dev->CreateTexture2D(&desc, nullptr, &rt.texture));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format                    = fmt;
    srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels       = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;
    GW2_CHECKED_HRESULT(dev->CreateShaderResourceView(rt.texture.Get(), &srvDesc, &rt.srv));

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    rtvDesc.Format             = fmt;
    rtvDesc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;
    GW2_CHECKED_HRESULT(dev->CreateRenderTargetView(rt.texture.Get(), &rtvDesc, &rt.rtv));

    return rt;
}

void DrawText(ID3D11DeviceContext* ctx, RenderTarget& rt, ID3D11BlendState* blendState, ImFont* font, float fontSize, const std::wstring& text)
{
    const uint  fgColor = 0xFFFFFFFF;
    const uint  bgColor = 0x00000000;

    const auto& txt     = utf8_encode(text);

    auto        sz      = font->CalcTextSizeA(fontSize, FLT_MAX, 0.f, txt.c_str());

    ImVec2      clip(1024.f, fontSize);

    float       xOff = (clip.x - sz.x) * 0.5f;

    ImDrawList  imDraw(ImGui::GetDrawListSharedData());
    imDraw.AddDrawCmd();
    imDraw.PushClipRect(ImVec2(0.f, 0.f), clip);
    imDraw.PushTextureID(font->ContainerAtlas->TexID);
    imDraw.AddText(font, fontSize, ImVec2(xOff, 0.f), fgColor, txt.c_str());

    auto& io             = ImGui::GetIO();
    auto  oldDisplaySize = io.DisplaySize;
    io.DisplaySize       = clip;

    ComPtr<ID3D11RenderTargetView> oldRt;
    ComPtr<ID3D11DepthStencilView> oldDs;
    ctx->OMGetRenderTargets(1, oldRt.GetAddressOf(), oldDs.GetAddressOf());
    ctx->OMSetRenderTargets(1, rt.rtv.GetAddressOf(), nullptr);

    float clearBlack[] = { 0.f, 0.f, 0.f, 0.f };
    ctx->ClearRenderTargetView(rt.rtv.Get(), clearBlack);

    ImDrawList* imDraws[] = { &imDraw };
    ImDrawData  imData;
    imData.Valid         = true;
    imData.CmdLists      = imDraws;
    imData.CmdListsCount = 1;
    imData.TotalIdxCount = imDraw.IdxBuffer.Size;
    imData.TotalVtxCount = imDraw.VtxBuffer.Size;
    imData.DisplayPos    = ImVec2(0.0f, 0.0f);
    imData.DisplaySize   = io.DisplaySize;

    {
        ImGuiBlendStateOverride ov(blendState);
        ImGui_ImplDX11_RenderDrawData(&imData);
    }

    ctx->OMSetRenderTargets(1, oldRt.GetAddressOf(), oldDs.Get());

    io.DisplaySize = oldDisplaySize;
}

Texture2D LoadCustomTexture(const std::filesystem::path& path)
{
    if (!FileSystem::Exists(path))
    {
        FormattedMessageBox(L"Could not load custom radial menu image '%s': file not found.", L"Custom Menu Error", path.wstring().c_str());
        return {};
    }

    const auto& data = FileSystem::ReadFile(path);
    if (data.empty())
    {
        FormattedMessageBox(L"Could not load custom radial menu image '%s': file is empty.", L"Custom Menu Error", path.wstring().c_str());
        return {};
    }

    try
    {
        auto                   dev = Core::i().device();
        Texture2D              tex;
        ComPtr<ID3D11Resource> res;
        HRESULT                hr = S_OK;
        if (path.extension() == L".dds")
            hr = DirectX::CreateDDSTextureFromMemory(dev.Get(), data.data(), data.size(), &res, &tex.srv);
        else
        {
            Core::i().RunCOMTask([&] { hr = DirectX::CreateWICTextureFromMemory(dev.Get(), data.data(), data.size(), &res, &tex.srv); });
        }
        if (res)
            res->QueryInterface(tex.texture.GetAddressOf());

        if (!SUCCEEDED(hr))
            FormattedMessageBox(L"Could not load custom radial menu image '%s': 0x%x.", L"Custom Menu Error", path.wstring().c_str(), hr);

        return tex;
    }
    catch (...)
    {
        FormattedMessageBox(L"Could not load custom radial menu image '%s'.", L"Custom Menu Error", path.wstring().c_str());
        return {};
    }
}

CustomWheelsManager::CustomWheelsManager(std::shared_ptr<Texture2D> bgTex, std::vector<std::unique_ptr<Wheel>>& wheels, ImFont* font)
    : wheels_(wheels)
    , font_(font)
    , backgroundTexture_(bgTex)
{
    CD3D11_BLEND_DESC blendDesc(D3D11_DEFAULT);
    blendDesc.RenderTarget[0].BlendEnable    = true;
    blendDesc.RenderTarget[0].SrcBlend       = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend      = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    GW2_CHECKED_HRESULT(Core::i().device()->CreateBlendState(&blendDesc, textBlendState_.GetAddressOf()));
}

void CustomWheelsManager::DrawOffscreen(ID3D11DeviceContext* ctx)
{
    if (!loaded_)
        Reload();
    else if (!textDraws_.empty())
    {
        auto& td = textDraws_.front();
        DrawText(ctx, td.rt, textBlendState_.Get(), font_, td.size, td.text);
        textDraws_.pop_front();
    }
}


void CustomWheelsManager::Draw(ID3D11DeviceContext* ctx)
{
    if (failedLoads_.empty())
        return;

    const auto& err = failedLoads_.back();

    ImGuiPopup("Custom wheel configuration could not be loaded!")
        .Position({ 0.5f, 0.45f })
        .Size({ 0.35f, 0.2f })
        .Display(
            [err](const ImVec2&)
            {
                ImGui::Text("Could not load custom wheel. Error message:");
                ImGui::TextWrapped(utf8_encode(err).c_str());
            },
            [&]() { failedLoads_.pop_back(); });
}

std::unique_ptr<Wheel> CustomWheelsManager::BuildWheel(const std::filesystem::path& configPath)
{
    const auto& dataFolder = configPath.parent_path();

    auto        fail       = [&](const wchar_t* error)
    {
        failedLoads_.push_back(error + (L": '" + configPath.wstring() + L"'"));
        return nullptr;
    };

    const auto& cfgSource = FileSystem::ReadFile(configPath);
    if (cfgSource.empty())
        return nullptr;

    CSimpleIniA ini(true);
    const auto& loadResult = ini.LoadData(reinterpret_cast<const char*>(cfgSource.data()), cfgSource.size());
    if (loadResult != SI_OK)
        return fail(L"Invalid INI file");

    const auto& general          = *ini.GetSection("General");
    const auto& wheelDisplayName = general.find("display_name");
    const auto& wheelNickname    = general.find("nickname");

    if (wheelDisplayName == general.end())
        return fail(L"Missing field display_name");
    if (wheelNickname == general.end())
        return fail(L"Missing field nickname");

    if (std::any_of(customWheels_.begin(), customWheels_.end(), [&](const auto& w) { return w->nickname() == wheelNickname->second; }))
        return fail((L"Nickname " + utf8_decode(std::string(wheelNickname->second)) + L" already exists").c_str());

    auto                          wheel  = std::make_unique<Wheel>(backgroundTexture_, wheelNickname->second, wheelDisplayName->second);

    uint                          baseId = customWheelNextId_;

    std::list<CSimpleIniA::Entry> sections;
    ini.GetAllSections(sections);
    sections.sort(CSimpleIniA::Entry::LoadOrder());
    std::vector<CustomElementSettings> elements;
    elements.reserve(sections.size());
    float maxTextWidth = 0.f;
    for (const auto& sec : sections)
    {
        if (_stricmp(sec.pItem, "General") == 0)
            continue;

        const auto& element                   = *ini.GetSection(sec.pItem);

        const auto& elementName               = element.find("name");
        const auto& elementColor              = element.find("color");
        const auto& elementIcon               = element.find("icon");
        const auto& elementShadow             = element.find("shadow_strength");
        const auto& elementColorize           = element.find("colorize_strength");
        const auto& elementPremultipliedAlpha = element.find("premultiply_alpha");
        const auto& elementProps              = element.find("props");

        glm::vec4   color{ 1.f };
        if (elementColor != element.end())
        {
            std::array<std::string, 3> colorElems;
            SplitString(elementColor->second, ",", colorElems.begin());

            size_t i = 0;
            for (const auto& c : colorElems)
                reinterpret_cast<float*>(&color)[i++] = static_cast<float>(atof(c.c_str()) / 255.f);
        }

        CustomElementSettings ces;
        ces.category    = wheel->nickname();
        ces.nickname    = ToLower(wheel->nickname()) + "_" + ToLower(std::string(sec.pItem));
        ces.name        = elementName == element.end() ? sec.pItem : elementName->second;
        ces.color       = color;
        ces.id          = baseId++;
        ces.shadow      = elementShadow == element.end() ? 1.f : static_cast<float>(atof(elementShadow->second));
        ces.colorize    = elementColorize == element.end() ? 1.f : static_cast<float>(atof(elementColorize->second));
        ces.premultiply = false;
        ces.props =
            elementProps == element.end() ? ConditionalProperties::USABLE_ALL | ConditionalProperties::VISIBLE_ALL : static_cast<ConditionalProperties>(atoi(elementProps->second));

        if (elementIcon != element.end())
        {
            ces.rt = LoadCustomTexture(dataFolder / utf8_decode(elementIcon->second));
            if (elementPremultipliedAlpha != element.end())
                ces.premultiply = ini.GetBoolValue(sec.pItem, "premultiply_alpha", true);
        }

        if (!ces.rt.texture)
            maxTextWidth = std::max(maxTextWidth, CalcText(font_, utf8_decode(ces.name)));

        elements.push_back(ces);

        GW2_ASSERT(baseId < customWheelNextId_ + CustomWheelIdStep);
    }

    float desiredFontSize = 1024.f / maxTextWidth * 100.f;

    for (auto& ces : elements)
    {
        if (!ces.rt.texture)
        {
            ces.rt = MakeTextTexture(desiredFontSize);
            textDraws_.push_back({ desiredFontSize, utf8_decode(ces.name), ces.rt });
        }

        auto we = std::make_unique<WheelElement>(ces.id, ces.nickname, ces.category, ces.name, ces.color, ces.props, ces.rt);
        we->shadowStrength(ces.shadow);
        we->colorizeAmount(ces.colorize);
        we->premultiplyAlpha(ces.premultiply);
        wheel->AddElement(std::move(we));
    }

    customWheelNextId_ += CustomWheelIdStep;

    return std::move(wheel);
}

void CustomWheelsManager::Reload()
{
    failedLoads_.clear();
    textDraws_.clear();
    customWheelNextId_ = CustomWheelStartId;

    if (!customWheels_.empty())
    {
        std::erase_if(wheels_, [&](const auto& ptr) { return std::find(customWheels_.begin(), customWheels_.end(), ptr.get()) != customWheels_.end(); });

        customWheels_.clear();
    }

    auto folderBaseOpt = INIConfigurationFile::i().folder();
    if (folderBaseOpt)
    {
        auto folderBase = *folderBaseOpt / L"custom";

        if (exists(folderBase))
        {
            auto addWheel = [&](const std::filesystem::path& configFile)
            {
                auto wheel = BuildWheel(configFile);
                if (wheel)
                {
                    wheels_.push_back(std::move(wheel));
                    customWheels_.push_back(wheels_.back().get());
                }
            };

            for (const auto& entry : std::filesystem::directory_iterator(folderBase))
            {
                if (!entry.is_directory() && entry.path().extension() != L".zip")
                    continue;

                std::filesystem::path configFile = entry.path() / L"config.ini";
                if (FileSystem::Exists(configFile))
                    addWheel(configFile);
                else if (auto dirs = FileSystem::IterateZipFolders(entry.path()); !dirs.empty())
                {
                    for (const auto& subdir : dirs)
                    {
                        std::filesystem::path subdirCfgFile = subdir / L"config.ini";
                        if (FileSystem::Exists(subdirCfgFile))
                            addWheel(subdirCfgFile);
                    }
                }
            }
        }
    }

    loaded_ = true;
}
} // namespace GW2Radial
