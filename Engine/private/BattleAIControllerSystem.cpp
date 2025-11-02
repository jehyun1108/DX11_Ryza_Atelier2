#include "Enginepch.h"

namespace
{
	static double elapsedTime = 0.0;

	inline double SecondUntilNextEval(float evalHz)
	{
		if (evalHz <= 0.0f) return 0.0;
		return 1.0 / static_cast<double>(evalHz);
	}
}

// ---------------------------------------------------------------------------------

void BattleAIControllerSystem::Update(float dt)
{
	elapsedTime += static_cast<double>(dt);

	const vector<EntityID> controllable = CollectEntities();
	if (controllable.empty()) return;

	auto& timelineSys = registry.Get<BattleTimelineSystem>();

	for (EntityID entity : controllable)
	{
		if (!ShouldEval(entity, elapsedTime)) continue;
		if (!timelineSys.IsUnitReadyToAct(entity)) continue;

		const EntityID resolvedTarget = ResolveTargetFirstEnemy(entity);
		if (resolvedTarget == invalidEntity) continue;

		TimelineActionIntent intent{};
		if (!BuildBasicIntent(entity, resolvedTarget, intent)) continue;

		(void)timelineSys.TryCommitIntent(entity, intent);
	}
}

vector<EntityID> BattleAIControllerSystem::CollectEntities() const
{
	vector<EntityID> result;
	result.reserve(6);

	auto& sessionSys = registry.Get<BattleSessionSystem>();

	const BattleParty*   allies  = sessionSys.GetAllies();
	const BattleEnemies* enemies = sessionSys.GetEnemies();
	const EntityID       leader  = sessionSys.GetLeader();

	if (enemies)
	{
		for (int i = 0; i < enemies->memberCount; ++i)
		{
			const EntityID enemy = enemies->members[i];
			if (enemy != invalidEntity)
				result.push_back(enemy);
		}
	}
	if (allies)
	{
		for (int i = 0; i < allies->memberCount; ++i)
		{
			const EntityID ally = allies->members[i];
			if (ally != invalidEntity && ally != leader)
				result.push_back(ally);
		}
	}
	return result;
}

bool BattleAIControllerSystem::ShouldEval(EntityID id, double now)
{
	AIBlackboard& bb = blackboard[id];
	if (now < bb.next_eval_sec) return false;

	const double interval = SecondUntilNextEval(config.eval_hz);
	bb.next_eval_sec = now + interval;
	return true;
}

bool BattleAIControllerSystem::IsGaugeFull(EntityID id) const
{
	auto& timelineSys = registry.Get<BattleTimelineSystem>();
	return timelineSys.IsGaugeFull(id);
}

EntityID BattleAIControllerSystem::ResolveTargetFirstEnemy(EntityID self) const
{
	auto& sessionSys = registry.Get<BattleSessionSystem>();

	BattleTeam selfTeam{};
	if (!sessionSys.TryGetTeam(self, selfTeam))
		return invalidEntity;

	const BattleParty*   allies  = sessionSys.GetAllies();
	const BattleEnemies* enemies = sessionSys.GetEnemies();

	if (selfTeam == BattleTeam::Ally)
	{
		if (enemies && enemies->memberCount > 0 && enemies->members[0] != invalidEntity)
			return enemies->members[0];
	}
	else if (selfTeam == BattleTeam::Enemy)
	{
		if (allies && allies->memberCount > 0 && allies->members[0] != invalidEntity)
			return allies->members[0];
	}
	return invalidEntity;
}

bool BattleAIControllerSystem::BuildBasicIntent(EntityID self, EntityID target, TimelineActionIntent& out) const
{
	if (self == invalidEntity || target == invalidEntity) return false;

	out              = {};
	out.battleCmd    = BattleCommand::AttackBasic;
	out.targetEntity = target;
	out.apCost       = 0;
	out.specialTag   = SpecialAnimTag::BasicAttack;
	return true;
}