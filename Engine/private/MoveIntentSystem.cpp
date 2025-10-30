#include "Enginepch.h"

Handle MoveIntentSystem::Create(EntityID owner)
{
	Handle handle = CreateComp(owner);
	if (auto intent = Get(handle))
		*intent = MoveIntent{};
	return handle;
}

void MoveIntentSystem::Clear(EntityID owner)
{
	if (auto intent = GetByOwner(owner))
		*intent = MoveIntent{};
}

void MoveIntentSystem::SetIntent(EntityID owner, const MoveIntent& in)
{
	if (auto intent = GetByOwner(owner))
		*intent = in;
}