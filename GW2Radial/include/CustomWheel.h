#pragma once

#include <filesystem>
#include <Main.h>
#include <Wheel.h>

namespace GW2Radial
{
    class CustomWheelsManager
    {
        std::vector<std::unique_ptr<Wheel>>& wheels_;
        std::vector<Wheel*> customWheels_;
        std::vector<std::wstring> failedLoads_;
        static constexpr uint CustomWheelStartId = 10000;
        static constexpr uint CustomWheelIdStep = 1000;
        uint customWheelNextId_ = CustomWheelStartId;
        ImFont* font_ = nullptr;
        bool loaded_ = false;
        ComPtr<ID3D11BlendState> textBlendState_;
        std::shared_ptr<Texture2D> backgroundTexture_;

        std::unique_ptr<Wheel> BuildWheel(const std::filesystem::path& configPath, ID3D11Device* dev);

        struct QueuedTextDraw
        {
            float size;
            std::wstring text;
            RenderTarget rt;
        };
        std::list<QueuedTextDraw> textDraws_;

        void Reload(ID3D11Device* dev);

    public:
        CustomWheelsManager(ID3D11Device* dev, std::shared_ptr<Texture2D> bgTexture, std::vector<std::unique_ptr<Wheel>>& wheels, ImFont* font);

        void Draw(ID3D11DeviceContext* ctx);
        void DrawOffscreen(ID3D11Device* dev, ID3D11DeviceContext* ctx);
        void MarkReload() { loaded_ = false; }
    };

    struct CustomElementSettings
    {
        uint id;
        std::string nickname;
        std::string category;
        std::string name;
        fVector4 color;
        float shadow;
        float colorize;

        RenderTarget rt;
        bool premultiply;
    };

    class CustomElement : public WheelElement
    {
        fVector4 color_;

    public:
        CustomElement(const CustomElementSettings& ces, ID3D11Device* dev);

    protected:
	    fVector4 color() override;
    };
}
