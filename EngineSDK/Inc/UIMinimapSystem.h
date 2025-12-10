#pragma once

#include "UIMinimapDatah.h"

NS_BEGIN(Engine)

class ENGINE_DLL UIMinimapSystem : public ISystem
{
public:
	explicit UIMinimapSystem(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;
	void     Tick(float dt);

	void SetMode(MinimapMode newMode)    { mode = newMode; }
	void SetFieldTarget(Handle playerTf) { fieldTargetTf = playerTf; }
	void SetBattleTarget(const _float3& center, float radius);

	void SetFieldCam(Handle cam)  { fieldCam = cam; }
	void SetBattleCam(Handle cam) { battleCam = cam; }

	const CameraProxy& GetCamera() const;
	MinimapMode        GetMode()   const { return mode; }

	bool WorldToMiniUV(const _float3& world, float& outU, float& outV) const;
	bool WorldToMiniUV_Global(const _float3& pos, float& u, float& v) const;

private:
	Handle  fieldCam{};
	Handle  battleCam{};
	_float3 battleCenter{};
	float   battleRadius = 500.f;
	Handle  fieldTargetTf{};

	MinimapMode mode = MinimapMode::None;

private:
	SystemRegistry&      registry;
	TransformSystem*     tfSys{};
	CameraSystem*        camSys{};
	RenderTargetMinimap* rtMinimap{};

	mutable CameraProxy  cachedCam{};
};

NS_END