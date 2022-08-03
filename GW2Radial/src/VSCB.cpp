#include <VSCB.h>

namespace GW2Radial
{
ConstantBuffer<VSCB>& GetVSCB()
{
    static ConstantBuffer<VSCB> vscb = ShaderManager::i().MakeConstantBuffer<VSCB>();
    return vscb;
}
} // namespace GW2Radial