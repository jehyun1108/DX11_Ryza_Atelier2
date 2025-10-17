#pragma once

NS_BEGIN(Engine)

struct ENGINE_DLL IOwnsEntities
{
	virtual ~IOwnsEntities() = default;
	virtual void DestroyOwned(EntityID owner) = 0;
};

NS_END