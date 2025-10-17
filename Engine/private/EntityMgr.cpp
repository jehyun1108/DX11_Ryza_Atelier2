#include "Enginepch.h"

EntityID EntityMgr::Create()
{
	EntityID id{};
	if (!freeList.empty())
	{
		id = freeList.back();
		freeList.pop_back();
	}
	else
		id = nextID++;

	alive.insert(id);
	return id;
}

void EntityMgr::Destroy(EntityID id)
{
	if (id == invalidEntity) return;
	if (alive.erase(id) == 0) return; // ÀÌ¹Ì Á×À½
	registry.DestroyOwned(id);
	freeList.push_back(id);
}

void EntityMgr::DestroyDeferred(EntityID id)
{
	if (id == invalidEntity) return;
	if (alive.count(id) == 0) return;
	deferred.push_back(id);
}

void EntityMgr::FlushDestroy()
{
	if (deferred.empty()) return;

	sort(deferred.begin(), deferred.end());
	deferred.erase(unique(deferred.begin(), deferred.end()), deferred.end());

	for (EntityID id : deferred)
		Destroy(id);
	deferred.clear();
}

void EntityMgr::Clear()
{
	for (EntityID id : alive)
		registry.DestroyOwned(id);

	alive.clear();
	deferred.clear();
	freeList.clear();
	nextID = 1;
}