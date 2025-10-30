#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL IntentMerger
{
public:
	static void MergeAndApply(SystemRegistry& registry, const IntentSnapShot& snapShot);
	// 부분 MergeRule 추가

private:
	static void CollectTargets(unordered_set<EntityID>& out, const unordered_map<EntityID, IntentWrite>& map);
	static const IntentWrite* PickByPriority(const IntentSnapShot& snapShot, EntityID entity);
};

NS_END