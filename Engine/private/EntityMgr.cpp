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

	
	if (id >= sparseIndices.size())
		sparseIndices.resize(id + 1, invalidIdx);

	sparseIndices[id] = (_uint)aliveIndices.size();
	aliveIndices.push_back(id);

	return id;
}

void EntityMgr::Destroy(EntityID id)
{
	if (id == invalidEntity) return;
	if (!IsAlive(id)) return;

	const _uint aliveIdxToRemove = sparseIndices[id];
	const EntityID lastId = aliveIndices.back();

	aliveIndices[aliveIdxToRemove] = lastId;
	sparseIndices[lastId] = aliveIdxToRemove;
	aliveIndices.pop_back();
	sparseIndices[id] = invalidIdx;

	registry.DestroyOwned(id);
	freeList.push_back(id);
}

void EntityMgr::DestroyDeferred(EntityID id)
{
	if (id == invalidEntity) return;
	if (!IsAlive(id)) return;
	deferred.push_back(id);
}

bool EntityMgr::IsAlive(EntityID id) const
{
	if (id == invalidEntity) return false;
	if (id >= sparseIndices.size()) return false;
	const _uint aliveIdx = sparseIndices[id];
	if (aliveIdx == invalidIdx) return false;
	if (aliveIdx >= aliveIndices.size()) return false;
	return aliveIndices[aliveIdx] == id;
}

void EntityMgr::Reserve(size_t n)
{
	freeList.reserve(n);
	aliveIndices.reserve(n);
	sparseIndices.reserve(n);
	deferred.reserve(n);
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
	for (EntityID id : aliveIndices)
		registry.DestroyOwned(id);

	aliveIndices.clear();
	sparseIndices.clear();
	deferred.clear();
	freeList.clear();
	nextID = 1;
}