#pragma once

#include "MoveStateData.h"

NS_BEGIN(Engine)

class ENGINE_DLL MoveStateSystem : public EntitySystem<MoveState>, public IGuiRenderable
{
public:
	explicit MoveStateSystem(SystemRegistry& registry) : EntitySystem(registry) {}
	
	Handle Create(EntityID owner, Handle tfHandle);
	void   Update(float dt);

	void RenderGui(EntityID filterID) override;
};

NS_END