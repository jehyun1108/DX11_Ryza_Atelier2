#pragma once

#include "SkyboxData.h"

NS_BEGIN(Engine)

class ENGINE_DLL SkyboxSystem : public EntitySystem<SkyboxState>, public IGuiRenderable
{
public:
	explicit SkyboxSystem(SystemRegistry& registry) : EntitySystem(registry) {}
	void     OnBoot() override;

	Handle Create(EntityID owner, Handle tfHandle, const vector<SkySubmesh>& submeshList, SkyTextureType texType = SkyTextureType::Equirect2D, bool attachToCam = true, float uniformScale = 1000.f, float baseYawRad = 0.f, float rotSpeed = 0.02f);

	void   SetActive(Handle handle, bool enable = true);
	Handle GetActive() const { return activeHandle; }
	void   Tick(float dt);
	
	void ExtractSkyboxProxies(SkyboxProxy& out) const;
	void ExtractFadeProxies(optional<SkyboxProxy>& outFrom, optional<SkyboxProxy>& outTo, float& outBlendWeight) const;

	// Utility 
	void SetAttachToCam(Handle handle, bool attach);
	void SetUniformScale(Handle handle, float scale);
	void SetBaseYaw(Handle handle, float baseYawRad);
	void SetRotSpeed(Handle handle, float rotSpeed);
	void SetPhase(Handle handle, float phaseRad);
	void SetSubmeshes(Handle handle, const vector<SkySubmesh>& submeshList);
	void SetTextureType(Handle handle, SkyTextureType type);

	// CrossFade
	void  StartCrossFade(Handle fromHandle, Handle toHandle, float dur);
	bool  IsCrossingFade()       const { return crossFade.active; }
	float GetCrossFadeProgress() const { return Utility::Saturate(crossFade.progress01); }

	void  RenderGui(EntityID id) override;

private:
	bool FindById(EntityID owner, Handle& outHandle, const SkyboxState*& outPtr) const;
	bool FindById(EntityID owner, Handle& outHandle, SkyboxState*& outPtr);
	void AdvancePhase(EntityID owner, float dt);

private:
	TransformSystem* tfSys{};

	Handle activeHandle{};
	SkyboxCrossFade crossFade{};
};

NS_END