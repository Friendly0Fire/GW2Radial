#include <ActivationKeybind.h>
#include <Input.h>

namespace GW2Radial
{
ActivationKeybind::~ActivationKeybind()
{
    Input::i().UnregisterKeybind(this);
}

void ActivationKeybind::Bind()
{
    Input::i().RegisterKeybind(this);
}
}
