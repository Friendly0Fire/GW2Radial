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

        std::unique_ptr<Wheel> BuildWheel(const std::filesystem::path& configPath, IDirect3DDevice9* dev);

        struct QueuedTextDraw
        {
            float size;
            std::wstring text;
            IDirect3DTexture9* tex;
        };
        std::list<QueuedTextDraw> textDraws_;

        void Reload(IDirect3DDevice9* dev);

    public:
        CustomWheelsManager(std::vector<std::unique_ptr<Wheel>>& wheels, ImFont* font);

        void Draw(IDirect3DDevice9* dev);
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

        IDirect3DTexture9* texture = nullptr;
    };

    class CustomElement : public WheelElement
    {
        fVector4 color_;

    public:
        CustomElement(const CustomElementSettings& ces, IDirect3DDevice9* dev);

    protected:
	    fVector4 color() override;
    };
}
