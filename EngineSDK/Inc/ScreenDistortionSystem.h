#pragma once

#include "ScreenDistortionData.h"

NS_BEGIN(Engine)

class ENGINE_DLL ScreenDistortionSystem : public ISystem
{
public:
	explicit ScreenDistortionSystem(SystemRegistry& registry) : registry(registry) {}
	
	void OnBoot() override;
	void Tick(float dt);

	void StartBattleToField(const _float3& centerWorld);
	bool IsActive() const { return state.active; }

	void ExtractDistortionProxies(DistortionCB& cb) { cb = state.cb; }

private:
	ScreenDistortionState state;

private:
	SystemRegistry& registry;
	CameraSystem* camSys{};
};

NS_END