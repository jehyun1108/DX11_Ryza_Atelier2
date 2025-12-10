#pragma once

NS_BEGIN(Client)

class CamSpawner
{
public:
	explicit CamSpawner(SystemRegistry& registry);

public:
	EntityHandles SpawnFreeCam(const _float3& pos, float fovYDeg = 60.f, float nearZ = 3.f, float farZ = 10000.f, float moveSpeed = 500.f, bool makeMain = true);
	EntityHandles SpawnFieldOrbitCam(Handle targetTf, float fovDeg = 90.f, float initYaw = 0.f, float initPitch = 15.f, float initDist = 350.f, float nearZ = 10.f, float farZ = 10000.f, bool makeMain = true);
	EntityHandles SpawnFieldMiniCam();

private:
	SystemRegistry& registry;
	EntitySpawner*  spawner{};
	CameraSystem*   camSys{};
};

NS_END