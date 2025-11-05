#pragma once

#include "BattleAIData.h"

NS_BEGIN(Engine)

class ENGINE_DLL BattleAIControllerSystem : public ISystem
{
public:
	explicit BattleAIControllerSystem(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;
	void     Update(float dt);
	void     SetConfig(const AIConfig& config) { this->config = config; }

	const AIConfig& GetConfig() const          { return config; }

private:
	vector<EntityID> CollectEntities() const;
	bool             ShouldEval(EntityID id, double now);
	bool             IsGaugeFull(EntityID id) const               { return timelineSys->IsGaugeFull(id); }
	EntityID         ResolveTargetFirstEnemy(EntityID self) const;
	bool             BuildBasicIntent(EntityID self, EntityID target, TimelineActionIntent& out) const;

	EntityID         ResolveTargetViaSystem(EntityID self) const;

private:
	SystemRegistry& registry;
	BattleTimelineSystem* timelineSys{};
	BattleTargetSystem*   targetSys{};
	BattleSessionSystem*  sessionSys{};


	AIConfig        config{};
	unordered_map<EntityID, AIBlackboard> blackboard;
};

NS_END