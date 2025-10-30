#include "Enginepch.h"

void IntentMerger::MergeAndApply(SystemRegistry& registry, const IntentSnapShot& snapShot)
{
	unordered_set<EntityID> allTargets;
	CollectTargets(allTargets, *snapShot.script);
	CollectTargets(allTargets, *snapShot.manual);
	CollectTargets(allTargets, *snapShot.ai);

	auto& intentSys = registry.Get<MoveIntentSystem>();
	for (EntityID entity : allTargets)
	{
		const IntentWrite* picked = PickByPriority(snapShot, entity);
		if (picked)
			intentSys.SetIntent(entity, picked->intent);
	}
}

void IntentMerger::CollectTargets(unordered_set<EntityID>& out, const unordered_map<EntityID, IntentWrite>& map)
{
	for (const auto& pair : map)
		out.insert(pair.first);
}

// 고정규칙 Script > Manual > AI
const IntentWrite* IntentMerger::PickByPriority(const IntentSnapShot& snapShot, EntityID entity)
{
	if (auto it = snapShot.script->find(entity); it != snapShot.script->end())
		return &it->second;

	if (auto it = snapShot.manual->find(entity); it != snapShot.manual->end()) 
		return &it->second;

	if (auto it = snapShot.ai->find(entity); it != snapShot.ai->end()) 
		return &it->second;

	return nullptr;
}