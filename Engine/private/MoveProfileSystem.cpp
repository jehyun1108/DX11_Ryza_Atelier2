#include "Enginepch.h"

Handle MoveProfileSystem::Create(EntityID owner, const MoveProfile& preset)
{
	Handle handle = CreateComp(owner);
	if (auto profile = Get(handle))
		*profile = preset;
	return handle;
}

void MoveProfileSystem::SetWalkSpeed(Handle handle, float walkSpeed)
{
	if (auto profile = Get(handle))
		profile->walkSpeed = walkSpeed;
}

void MoveProfileSystem::SetRunSpeed(Handle handle, float runSpeed)
{
	if (auto profile = Get(handle))
		profile->runSpeed = runSpeed;
}
