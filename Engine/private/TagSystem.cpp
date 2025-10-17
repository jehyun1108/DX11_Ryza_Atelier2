#include "Enginepch.h"

void TagSystem::Register(EntityID entity, string tag)
{
	if (auto it = reverse.find(entity); it != reverse.end())
		Unregister(entity);

	const TagID id = HashTag(tag);

	if (auto it = byID.find(id); it != byID.end())
	{
		if (it->second == entity) return;
#ifdef _DEBUG
		auto itStr = debugMap.find(id);
		if (itStr != debugMap.end() && itStr->second != tag)
			assert(false && "TagSystem: FNV-1a collision detected for different tags");
		assert(false && "TagSystem: tag already used by another entity");
		return;
#endif
	}
	byID[id] = entity;
	reverse[entity] = id;
#ifdef _DEBUG
	debugMap[id] = move(tag);
#endif
}

void TagSystem::Unregister(EntityID entity)
{
	auto it = reverse.find(entity);
	if (it == reverse.end()) return;

	const TagID id = it->second;

	if (auto itID = byID.find(id); itID != byID.end() && itID->second == entity)
		byID.erase(itID);

#ifdef _DEBUG
	if (auto itStr = debugMap.find(id); itStr != debugMap.end())
		debugMap.erase(itStr);
#endif
	reverse.erase(it);
}

EntityID TagSystem::Get(TagID tag) const
{
	auto it = byID.find(tag);
	return (it == byID.end()) ? invalidEntity : it->second;
}

EntityID TagSystem::Get(const string& tag) const
{
	return Get(HashTag(tag));
}

void TagSystem::DestroyOwned(EntityID owner)
{
	Unregister(owner);
}