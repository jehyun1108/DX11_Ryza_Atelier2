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

	camSys->SetFollowOffsetSpace(camHandle, OffsetSpace::WorldSpace);
	camSys->SetFollowPolicy(camHandle, FollowPolicy::HardLookAt);

	_float3 initOffset = { 0.0f, 2.0f , -orbit.orbitDist }; 
	camSys->SetTarget(camHandle, targetTf, XMLoadFloat3(&initOffset));
	_float3 lookAtOffset = { 0.f, orbit.lookAtOffsetY, 0.f };
	camSys->SetLookAtOffset(camHandle, XMLoadFloat3(&lookAtOffset));

	return handle;
}

void OrbitCamSystem::Update(float dt)
{
	ForEachAliveEx([&](Handle handle, EntityID owner, OrbitCamData& orbit)
		{
			if (!orbit.isActive) return;

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

			float t = (orbit.orbitPitch - orbit.minPitch) / (orbit.maxPitch - orbit.minPitch);
			t = clamp(t, 0.f, 1.f);

			const float bottomFactor = 0.3f;
			float minDistBottom = orbit.minDist * bottomFactor;
			float minDistTop    = orbit.minDist;
			float curDist       = orbit.maxDist;

			if (t <= 0.5f)
			{
				float tb = t / 0.5f;
				float edge = 1.f - tb;
				curDist = orbit.maxDist + (minDistBottom - orbit.maxDist) * edge;
			}
			else
			{
				float tt = (t - 0.5f) / 0.5f;
				float edge = tt;
				curDist = orbit.maxDist + (minDistTop - orbit.maxDist) * edge;
			}

			_float3 orbitDir     = { cosPitch * sinYaw, sinPitch, cosPitch * cosYaw };
			_float3 followOffset = { orbitDir.x * curDist, orbitDir.y * curDist, orbitDir.z * curDist };

			camSys->SetTarget(orbit.camHandle, orbit.targetTf, XMLoadFloat3(&followOffset));
		});
}