#pragma once

#include "GroundingData.h"

NS_BEGIN(Engine)

class ENGINE_DLL GroundingSystem : public EntitySystem<GroundingComponent>, public IGuiRenderable
{
public: 
	explicit GroundingSystem(SystemRegistry& registry) : EntitySystem(registry) {}
	void     OnBoot() override;
	Handle   Create(EntityID owner, const GroundingComponent& init = {});

	void SetEnabled(Handle handle, bool on);
	void SetParams(Handle handle, const GroundingComponent& p);
	void Tick(float dt);

	void RenderGui(EntityID id);

private:
	CollisionSystem*   collisionSys{};
	TransformSystem*   tfSys{};
	MoveStateSystem*   moveSys{};
	NavMeshSystem*     navSys{};
};

NS_END