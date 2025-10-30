#pragma once

NS_BEGIN(Engine)

struct RunInJob
{
	EntityID entity{};
	_float3  targetPos{};
	float    targetYawDeg = 0.f;
	
	float    arriveDist = 10.f;
	float    stopSpeed = 1.f;
	bool     reached = false;
};

NS_END