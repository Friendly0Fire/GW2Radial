#pragma once

#include "ShaderManager.h"

namespace GW2Radial
{
	struct VSCB
	{
		fVector4 spriteDimensions;
	};

	ConstantBuffer<VSCB>& GetVSCB();
}