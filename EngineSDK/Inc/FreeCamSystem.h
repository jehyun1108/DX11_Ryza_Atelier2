#pragma once

#include "FreeCamData.h"

NS_BEGIN(Engine)

class ENGINE_DLL FreeCamSystem : public EntitySystem<FreeCamData>, public IGuiRenderable
{
public:
	explicit FreeCamSystem(SystemRegistry& registry) : EntitySystem(registry) {}

	Handle Create(EntityID owner, Handle transform, float moveSpeed = 200.f, float sens = 0.25f);

	void SetActive(Handle handle, bool on);
	void SetSpeed(Handle handle, float speed);
	void SetSensitivity(Handle handle, float sens);

	void Update(float dt);
	void RenderGui(EntityID id) override;
};

NS_END