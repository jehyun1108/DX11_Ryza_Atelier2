#pragma once

NS_BEGIN(Engine)

struct ENGINE_DLL MoveState
{
	Handle  tfHandle;

	_float2 velocityXZ = {};
	float   velocityY  = 0.f;

	bool    grounded     = false;
	bool    prevGrounded = false;
	_float3 groundNormal = { 0.f, 1.f, 0.f };
	float   groundY      = 0.f;

	bool    hasGround = false;
};

NS_END