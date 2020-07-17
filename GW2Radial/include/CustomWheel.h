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
        IDirect3DDevice9* device_;
        std::vector<std::wstring> failedLoads_;
        static constexpr uint CustomWheelStartId = 10000;
        static constexpr uint CustomWheelIdStep = 1000;
        uint customWheelNextId_ = CustomWheelStartId;
        ImFont* font_ = nullptr;

        std::unique_ptr<Wheel> BuildWheel(const std::filesystem::path& configPath);

    public:
        CustomWheelsManager(std::vector<std::unique_ptr<Wheel>>& wheels, ImFont* font, IDirect3DDevice9* dev);

        void Draw();
        void Reload();
    };

    struct CustomElementSettings
    {
        uint id;
        std::string nickname;
        std::string category;
        std::string name;
        fVector4 color;

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
