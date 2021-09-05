#include <ActivationKeybind.h>
#include <Input.h>

namespace GW2Radial
{
ActivationKeybind::~ActivationKeybind()
{
    Input::i([&](auto& i) { i.UnregisterKeybind(this); });
}

void ActivationKeybind::Bind()
{
    Input::i().RegisterKeybind(this);
}

void ActivationKeybind::Rebind()
{
    Input::i().UpdateKeybind(this);
}
}
