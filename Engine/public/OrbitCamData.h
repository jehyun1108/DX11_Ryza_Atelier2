#pragma once

NS_BEGIN(Engine)

struct OrbitCamData
{
	Handle camHandle{};
	Handle targetTf{};

	float orbitYaw   = 0.f;  // ÁÂ¿ì
	float orbitPitch = 15.f; // »óÇÏ
	float orbitDist  = 6.f;

	float minPitch = 20.f;
	float maxPitch = 60.f;
	float minDist  = 2.f;
	float maxDist  = 12.f;

	float yawSensitivity   = 100.f;
	float pitchSensitivity = 80.f;
	float zoomSensitivity  = 1.f;

	float lookAtOffset = 1.6f;
	bool  isActive     = true;
};

NS_END