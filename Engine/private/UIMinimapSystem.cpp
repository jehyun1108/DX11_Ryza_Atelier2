#include "Enginepch.h"
#include "UIMinimapSystem.h"

void UIMinimapSystem::OnBoot()
{
	tfSys     = &registry.Get<TransformSystem>();
	camSys    = &registry.Get<CameraSystem>();
	rtMinimap = &registry.Get<RenderTargetMinimap>();
}

void UIMinimapSystem::Tick(float dt)
{
	UNREFERENCED_PARAMETER(dt);
}

void UIMinimapSystem::SetBattleTarget(const _float3& center, float radius)
{
	battleCenter = center;
	battleRadius = max(radius, 1e-3f);
}

const CameraProxy& UIMinimapSystem::GetCamera() const
{
	Handle camHandle{};

	switch (mode)
	{
	case MinimapMode::Field:
		camHandle = fieldCam;
		break;

	case MinimapMode::Battle:
		camHandle = battleCam;
		break;

	case MinimapMode::None:
	default:
		camHandle = camSys->GetMainCamHandle();
		break;
	}

	camSys->ExtractCameraProxy(camHandle, cachedCam);
	return cachedCam;
}

bool UIMinimapSystem::WorldToMiniUV(const _float3& world, float& outU, float& outV) const
{
	if (mode == MinimapMode::None)
		return false;

	_float3 center{};
	float radius = 0.f;

	if (mode == MinimapMode::Battle)
	{
		center = battleCenter;
		radius = battleRadius;
	}
	else
	{
		radius = battleRadius;
		const TransformData* tf = tfSys->Get(fieldTargetTf);
		center = tf->pos;
	}

	radius = max(radius, 1e-3f);

	const float dx = world.x - center.x;
	const float dz = world.z - center.z;

	const float distSq = dx * dx + dz * dz;
	const float radiusSq = radius * radius;

	if (distSq > radiusSq)
		return false;

	const float invRadius = 1.f / radius;

	const float nx = dx * invRadius;
	const float ny = dz * invRadius;

	outU = 0.5f + nx * 0.5f;
	outV = 0.5f - ny * 0.5f;
	return true;
}

bool UIMinimapSystem::WorldToMiniUV_Global(const _float3& pos, float& u, float& v) const
{
	constexpr float mapAspect = 303.f / 584.f;
	constexpr float mapHalfWidth = 18000.f;
	constexpr float mapHalfHeight = mapHalfWidth * mapAspect;
	
	_float3 mapCenter = { mapHalfWidth * 0.2f , 0.f, -mapHalfHeight * 0.05f };

	float dx = pos.x - mapCenter.x;
	float dz = pos.z - mapCenter.z;

	u = 0.5f + dx / (2.f * mapHalfWidth);
	v = 0.5f - dz / (2.f * mapHalfHeight);
	return (u >= 0.f && u <= 1.f && v >= 0.f && v <= 1.f);
}