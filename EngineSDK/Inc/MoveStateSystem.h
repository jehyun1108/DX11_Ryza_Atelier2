#pragma once

#include "MoveStateData.h"

NS_BEGIN(Engine)

class ENGINE_DLL MoveStateSystem : public EntitySystem<MoveState>, public IGuiRenderable
{
public:
	explicit MoveStateSystem(SystemRegistry& registry) : EntitySystem(registry) {}
	void     OnBoot() override;

	Handle Create(EntityID owner, Handle tfHandle);
	void   Update(float dt);

	void RenderGui(EntityID id) override;

private:
	TransformSystem*   tfSys{};
	MoveProfileSystem* profileSys{};
	MoveIntentSystem*  intentSys{};
	NavMeshSystem*     navSys{};

	float castUp = 0.3f;
	float castDown = 3.f;
};

NS_END