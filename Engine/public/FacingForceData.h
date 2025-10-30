#pragma once

NS_BEGIN(Engine)

struct FacingForceRequest
{
	bool    has     = false;
	_float3 forward = { 0.f, 0.f, 1.f };
	bool    lockXZ  = true;
	bool    snap    = false;
};

NS_END