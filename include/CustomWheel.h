﻿#pragma once

#include <Main.h>
#include <Wheel.h>
#include <filesystem>

namespace GW2Radial
{
class CustomWheelsManager
{
    std::vector<std::unique_ptr<Wheel>>& wheels_;
    std::vector<Wheel*>                  customWheels_;
    std::vector<std::wstring>            failedLoads_;
    static constexpr u32                 CustomWheelStartId = 10000;
    static constexpr u32                 CustomWheelIdStep  = 1000;
    u32                                  customWheelNextId_ = CustomWheelStartId;
    ImFont*                              font_              = nullptr;
    bool                                 loaded_            = false;
    ComPtr<ID3D11BlendState>             textBlendState_;
    std::shared_ptr<Texture2D>           backgroundTexture_;

    std::unique_ptr<Wheel>               BuildWheel(const std::filesystem::path& configPath);

    struct QueuedTextDraw
    {
        float        size;
        std::wstring text;
        RenderTarget rt;
    };
    std::list<QueuedTextDraw> textDraws_;

    void                      Reload();

public:
    CustomWheelsManager(std::shared_ptr<Texture2D> bgTexture, std::vector<std::unique_ptr<Wheel>>& wheels, ImFont* font);

    void Draw(ID3D11DeviceContext* ctx);
    void DrawOffscreen(ID3D11DeviceContext* ctx);
    void MarkReload()
    {
        loaded_ = false;
    }
};

struct CustomElementSettings
{
    u32                   id;
    std::string           nickname;
    std::string           category;
    std::string           name;
    glm::vec4             color;
    float                 shadow;
    float                 colorize;
    ConditionalProperties props;

    RenderTarget          rt;
    bool                  premultiply;
};

class CustomWheel : public Wheel
{
public:
    CustomWheel(std::shared_ptr<Texture2D> bgTexture);
};
} // namespace GW2Radial
