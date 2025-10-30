#pragma once

#include "MovementData.h"

NS_BEGIN(Engine)

class ENGINE_DLL MovementSystem : public EntitySystem<MovementData>, public IGuiRenderable
{
public:
	explicit MovementSystem(SystemRegistry& registry, TransformSystem& tfSys) : EntitySystem(registry), tfSys(tfSys) {}

	Handle Create(EntityID owner, Handle tfHandle, const MovementData& initial = {});
	
	void SetIntent(Handle handle, const MoveIntent& intent);

	void Update(float dt);

	void RenderGui(EntityID id) override;

private:
	TransformSystem& tfSys;
};

NS_END