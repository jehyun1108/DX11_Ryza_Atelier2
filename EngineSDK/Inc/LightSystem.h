#pragma once

#include "LightData.h"

NS_BEGIN(Engine)

class ENGINE_DLL LightSystem : public EntitySystem<LightData>, public IGuiRenderable
{
public:
	explicit LightSystem(SystemRegistry& registry) : EntitySystem(registry) {}

	Handle Create(EntityID owner, Handle transform, const LightProxy& proxy = {});

	void Update(float dt);
	void ExtractLightProxies(vector<LightProxy>& out) const;

	void SetEnabled(Handle handle, bool on);
	void SetLightType(Handle handle, LIGHT type);
	void SetAmbient(Handle handle, _fvec color);
	void SetDiffuse(Handle handle, _fvec color);
	void SetSpecular(Handle handle, _fvec color);
	void SetRange(Handle handle, float range);
	void SetSpotAngle(Handle handle, float angle);
	void SetDir(Handle handle, _fvec dir);

	const LightProxy* GetProxy(Handle handle) const;
	void RenderGui(EntityID id) override;
};

NS_END