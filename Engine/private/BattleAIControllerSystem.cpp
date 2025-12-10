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
void BattleAIControllerSystem::OnBoot()
{
	timelineSys = &registry.Get<BattleTimelineSystem>();
	targetSys   = &registry.Get<BattleTargetSystem>();
	sessionSys  = &registry.Get<BattleSessionSystem>();
}

void BattleAIControllerSystem::Update(float dt)
{
	elapsedTime += static_cast<double>(dt);

	const vector<EntityID> controllable = CollectEntities();
	if (controllable.empty()) return;

	for (EntityID entity : controllable)
	{
		if (!IsInTimeline(entity))
			continue;

		if (!ShouldEval(entity, elapsedTime))
			continue;

		if (!timelineSys->IsUnitReadyToAct(entity))
			continue;

		EntityID targetEntity = targetSys->Get(entity);

		if (targetEntity == 0u)
			continue;
		if (!IsInTimeline(targetEntity))
			continue;

		TimelineActionIntent intent = {
			.battleCmd = BattleCommand::AttackBasic,
			.targetEntity = targetEntity,
			.apCost = 0,
			.specialTag = SpecialAnimTag::BasicAttack
		};

		timelineSys->CommitIntent(entity, intent);
	}
}

vector<EntityID> BattleAIControllerSystem::CollectEntities() const
{
	vector<EntityID> result;
	result.reserve(6);

	const BattleEnemies& enemies = sessionSys->GetEnemies();
	const BattleParty&   allies = sessionSys->GetAllies();
	const EntityID       leader = timelineSys->GetLeader();

	for (int i = 0; i < enemies.memberCount; ++i)
	{
		const EntityID enemy = enemies.members[i];
		if (enemy != 0u)
			result.push_back(enemy);
	}
	for (int i = 0; i < allies.memberCount; ++i)
	{
		const EntityID ally = allies.members[i];
		if (ally != 0u && ally != leader)
			result.push_back(ally);
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

EntityID BattleAIControllerSystem::ResolveTargetFirstEnemy(EntityID self) const
{
	const BattleTeam     selfTeam = sessionSys->GetTeam(self);
	const BattleParty&   allies   = sessionSys->GetAllies();
	const BattleEnemies& enemies  = sessionSys->GetEnemies();

	if (selfTeam == BattleTeam::Ally)
	{
		if (enemies.memberCount > 0 && enemies.members[0] != invalidEntity)
			return enemies.members[0];
	}
	else // Enemy
	{
		if (allies.memberCount > 0 && allies.members[0] != invalidEntity)
			return allies.members[0];
	}
	return invalidEntity;
}


EntityID BattleAIControllerSystem::ResolveTargetViaSystem(EntityID self) const
{
	return targetSys->Get(self);
}

bool BattleAIControllerSystem::IsInTimeline(EntityID id) const
{
	if (id == 0u) return false;

	const auto& state = timelineSys->GetState();

	for (int i = 0; i < state.alliesUsed; ++i)
		if (state.allies[i].entity == id)
			return true;

	for (int i = 0; i < state.enemiesUsed; ++i)
		if (state.enemies[i].entity == id)
			return true;

	return false;
}