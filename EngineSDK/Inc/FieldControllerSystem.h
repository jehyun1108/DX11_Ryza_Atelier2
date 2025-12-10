#pragma once

#include "FieldControllerData.h"

NS_BEGIN(Engine)

class ENGINE_DLL FieldControllerSystem : public EntitySystem<FieldControllerState>
{
public:
	explicit FieldControllerSystem(SystemRegistry& registry) :EntitySystem(registry) {}
	void     OnBoot() override;
	Handle   Create(EntityID leader, Handle camTf);
	void     Update(EntityID leader, float dt);

private:
	void  SubmitFieldMoveIntent(EntityID leader, FieldControllerState& state, float dt);

private:
	TransformSystem*       tfSys{};
	MoveStateSystem*       moveSys{};
	InputService*          input{};
	WorldMapPresenter*     worldMap{};
	FieldUIOrchestrator*   fieldUI{};
	EffectSystem*          effectSys{};
	DressingRoomPresenter* dressing{};
};

NS_END