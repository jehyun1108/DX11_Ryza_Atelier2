#pragma once

#include "MoveProfileData.h"

NS_BEGIN(Engine)

class ENGINE_DLL MoveProfileSystem : public EntitySystem<MoveProfile>
{
public:
	explicit MoveProfileSystem(SystemRegistry& registry) : EntitySystem(registry) {}

	Handle Create(EntityID owner, const MoveProfile& preset);

	void SetWalkSpeed(Handle handle, float walkSpeed);
	void SetRunSpeed(Handle handle, float runSpeed);
};

NS_END