#pragma once

#include <Main.h>
#include <Wheel.h>

namespace GW2Radial
{
    class CustomWheelsManager
    {
    public:
        CustomWheelsManager(std::vector<std::unique_ptr<Wheel>>& wheels, IDirect3DDevice9* dev);
    };

    struct CustomElementSettings
    {
        uint id;
        std::string nickname;
        std::string category;
        std::string name;
        fVector4 color;

        std::wstring texturePath;
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
