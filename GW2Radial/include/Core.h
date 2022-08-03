#pragma once

#include <CustomWheel.h>
#include <Defs.h>
#include <Direct3D11Loader.h>
#include <Main.h>
#include <Singleton.h>
#include <Wheel.h>
#include <d3d11_1.h>
#include <dxgi.h>

class ShaderManager;

namespace GW2Radial
{

class Core : public BaseCore, public Singleton<Core>
{
public:
    void ForceReloadWheels()
    {
        forceReloadWheels_ = true;
    }

    const auto& wheels() const
    {
        return wheels_;
    }

protected:
    void InnerDraw() override;
    void InnerUpdate() override;
    void InnerFrequentUpdate() override;
    void InnerOnFocus() override;
    void InnerOnFocusLost() override;
    void InnerInitPreImGui() override;
    void InnerInitPostImGui() override;
    void InnerInternalInit() override;
    void InnerShutdown() override;

    uint GetShaderArchiveID() const override
    {
        return IDR_SHADERS;
    }
    const wchar_t* GetShaderDirectory() const override
    {
        return SHADERS_DIR;
    }
    const wchar_t* GetGithubRepoSubUrl() const override
    {
        return L"Friendly0Fire/GW2Radial";
    }

    bool                                       forceReloadWheels_ = false;
    uint                                       mapId_             = 0;
    std::wstring                               characterName_;

    std::vector<std::unique_ptr<Wheel>>        wheels_;
    std::unique_ptr<CustomWheelsManager>       customWheels_;

    std::unique_ptr<ConfigurationOption<bool>> firstMessageShown_;

    std::shared_ptr<Texture2D>                 bgTex_;
};
} // namespace GW2Radial