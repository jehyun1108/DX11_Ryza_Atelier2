#pragma once

#include "JumpData.h"

NS_BEGIN(Engine)

class ENGINE_DLL JumpSystem : public EntitySystem<JumpComponent>
{
public:
	explicit JumpSystem(SystemRegistry& registry) : EntitySystem(registry) {}
	void     OnBoot() override;
	Handle   Create(EntityID owner);

	void SetParams(const JumpParams& params) { jumpParams = params; }
	JumpParams GetParams() const { return jumpParams; }

	void Priority_Update(float dt);
	void Update(float dt);

private:
	InputService*    input{};
	MoveStateSystem* moveSys{};

	JumpParams jumpParams;
};

NS_END