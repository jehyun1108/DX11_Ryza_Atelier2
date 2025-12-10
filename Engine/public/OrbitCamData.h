#pragma once

NS_BEGIN(Engine)

struct OrbitCamData
{
	Handle camHandle{};
	Handle targetTf{};

	float orbitYaw   = 0.f;  // ÁÂ¿ì
	float orbitPitch = 15.f; // »óÇÏ
	float orbitDist  = 6.f;

	float minPitch = -10.f;
	float maxPitch = 60.f;
	float minDist  = 200.f;
	float maxDist  = 400.f;

	float yawSensitivity   = 10.f;
	float pitchSensitivity = 8.f;
	float zoomSensitivity  = 1.f;

	float lookAtOffsetY = 100.f;
	bool  isActive      = true;
};

NS_END