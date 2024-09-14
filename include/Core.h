#pragma once

#include <CustomWheel.h>
#include <Defs.h>
#include <Main.h>
#include <Singleton.h>
#include <Wheel.h>
#include <Win.h>
#include <d3d11_1.h>
#include <dxgi.h>

class ShaderManager;

namespace GW2Radial
{

class Core : public BaseCore, public Singleton<Core>
{
public:
    struct VertexCB
    {
        glm::vec4   spriteDimensions;
        glm::mat4x4 tiltMatrix;
        float       spriteZ;
    };

    void ForceReloadWheels()
    {
        forceReloadWheels_ = true;
    }

    const auto& wheels() const
    {
        return wheels_;
    }

    void RunCOMTask(std::function<void()>&& fct)
    {
        {
            std::unique_lock lk(comTaskMutex_);
            comTask_ = std::move(fct);
        }
        comNotify_.notify_one();

        // We must block to wait for the task to end because D3D is in single threaded mode
        {
            std::unique_lock lk(comTaskMutex_);
            comNotify_.wait(lk);
        }
    }

    ConstantBufferSPtr<VertexCB> vertexCB()
    {
        return vertexCB_;
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

    u32  GetShaderArchiveID() const override
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
    u32                                        mapId_             = 0;
    std::wstring                               characterName_;

    std::vector<std::unique_ptr<Wheel>>        wheels_;
    std::unique_ptr<CustomWheelsManager>       customWheels_;

    std::unique_ptr<ConfigurationOption<bool>> firstMessageShown_;

    std::shared_ptr<Texture2D>                 bgTex_;
    ConstantBufferSPtr<VertexCB>               vertexCB_;

    std::unique_ptr<std::jthread>              comThread_;
    std::optional<std::function<void()>>       comTask_;
    std::mutex                                 comTaskMutex_;
    std::condition_variable                    comNotify_;
};
} // namespace GW2Radial