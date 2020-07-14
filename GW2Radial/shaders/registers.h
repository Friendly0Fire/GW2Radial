#pragma once

#ifndef _WINDOWS
#define PI 3.14159f
#define SQRT2 1.4142136f
#define ONE_OVER_SQRT2 0.707107f
#endif
#define WHEEL_MAX_ELEMENT_COUNT 9

#ifdef _WINDOWS
#define VALUE_REGISTER(type, name, slot, slotType, ns) namespace ShaderRegister::ns { const unsigned int type##_##name = slot; }
#define VALUE_REGISTER_ARRAY(type, name, sz, slot, slotType, ns) namespace ShaderRegister::ns { const unsigned int array_##type##_##name = slot; }
#else
#define VALUE_REGISTER(type, name, slot, slotType, ns) type name : register( slotType##slot );
#define VALUE_REGISTER_ARRAY(type, name, sz, slot, slotType, ns) type name[sz] : register( slotType##slot );
#endif

VALUE_REGISTER(sampler2D, texMainSampler, 0, s, ShaderPS);
VALUE_REGISTER(sampler2D, texWipeMaskImageSampler, 1, s, ShaderPS);

#if defined(_WINDOWS) || defined(SHADER_PS)
VALUE_REGISTER(float, fAnimationTimer, 0, c, ShaderPS);
VALUE_REGISTER(float, fWheelFadeIn, 1, c, ShaderPS);
VALUE_REGISTER(float, fCenterScale, 2, c, ShaderPS);
VALUE_REGISTER(int, iElementCount, 3, c, ShaderPS);
VALUE_REGISTER(float3, fWipeMaskData, 4, c, ShaderPS);
VALUE_REGISTER(int, iElementID, 5, c, ShaderPS);
VALUE_REGISTER(float4, fElementColor, 6, c, ShaderPS);
VALUE_REGISTER_ARRAY(float4, fHoverFadeIns, WHEEL_MAX_ELEMENT_COUNT, 7, c, ShaderPS);
#endif

#if defined(_WINDOWS) || defined(SHADER_VS)
VALUE_REGISTER(float4, fSpriteDimensions, 0, c, ShaderVS);
#endif