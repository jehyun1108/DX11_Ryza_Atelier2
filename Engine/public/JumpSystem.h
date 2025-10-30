#pragma once

#include "JumpData.h"

NS_BEGIN(Engine)

class ENGINE_DLL JumpSystem : public EntitySystem<JumpComponent>
{
public:
	explicit JumpSystem(SystemRegistry& registry) : EntitySystem(registry) {}
	Handle   Create(EntityID owner);

	void SetParams(const JumpParams& params) { jumpParams = params; }
	JumpParams GetParams() const { return jumpParams; }

	void Priority_Update(float dt);
	void Update(float dt);

private:
	JumpParams jumpParams;
};

NS_END