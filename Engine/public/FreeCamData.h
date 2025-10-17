#pragma once

NS_BEGIN(Engine)

struct ENGINE_DLL FreeCamData
{
	Handle transform;
	float moveSpeed   = 5.f;
	float sensitivity = 0.25f;
	bool  isActive    = true;

	float yawDeg = 0.f;
	float pitchDeg = 0.f;
};

NS_END