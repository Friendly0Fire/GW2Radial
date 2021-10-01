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
    if(notNone(key_))
        Input::i().RegisterKeybind(this);
}

void ActivationKeybind::Rebind()
{
    if (notNone(key_))
        Input::i().UpdateKeybind(this);
    else
        Input::i().UnregisterKeybind(this);
}
}
