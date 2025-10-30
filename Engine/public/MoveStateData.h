#pragma once

NS_BEGIN(Engine)

struct ENGINE_DLL MoveState
{
	Handle  tfHandle;

	_float2 velocityXZ = {};
	float   velocityY  = 0.f;

	bool    grounded     = false;
	_float3 groundNormal = { 0.f, 1.f, 0.f };
};

NS_END