#include "Enginepch.h"

void OrbitCamSystem::OnBoot()
{
	input  = &registry.Get<InputService>();
	camSys = &registry.Get<CameraSystem>();
}

Handle OrbitCamSystem::Create(EntityID owner, Handle camHandle, Handle targetTf)
{
	Handle handle = CreateComp(owner);
	auto& orbit = *Get(handle);
	orbit = {};
	orbit.camHandle = camHandle;
	orbit.targetTf = targetTf;

	auto& camSys = registry.Get<CameraSystem>();
	camSys.SetFollowOffsetSpace(camHandle, OffsetSpace::WorldSpace);
	camSys.SetFollowPolicy(camHandle, FollowPolicy::HardLookAt);

	_float3 initOffset = { 0.0f, 2.0f, -orbit.orbitDist }; 
	camSys.SetTarget(camHandle, targetTf, XMLoadFloat3(&initOffset));
	return handle;
}

void OrbitCamSystem::Update(float dt)
{
	ForEachAliveEx([&](Handle handle, EntityID owner, OrbitCamData& orbit)
		{
			if (!orbit.isActive) return;
			if (!camSys->Validate(orbit.camHandle)) return;

			const _float2 mouseDelta = input->GetMouseDelta();

			orbit.orbitYaw += (mouseDelta.x * orbit.yawSensitivity) * dt;
			orbit.orbitPitch += ((mouseDelta.y) * orbit.pitchSensitivity) * dt;
			orbit.orbitPitch = clamp(orbit.orbitPitch, orbit.minPitch, orbit.maxPitch);

			const float yawRad   = XMConvertToRadians(orbit.orbitYaw);
			const float pitchRad = XMConvertToRadians(orbit.orbitPitch);

			const float cosPitch = cosf(pitchRad);
			const float sinPitch = sinf(pitchRad);
			const float cosYaw   = cosf(yawRad);
			const float sinYaw   = sinf(yawRad);

			_float3 orbitDir     = { cosPitch * sinYaw, sinPitch, cosPitch * cosYaw };
			_float3 followOffset = { orbitDir.x * orbit.orbitDist, orbitDir.y * orbit.orbitDist, orbitDir.z * orbit.orbitDist };

			camSys->SetTarget(orbit.camHandle, orbit.targetTf, XMLoadFloat3(&followOffset));
		});
}