#include "Enginepch.h"

void BattleTimelineSystem::InitSession(const BattleSessionState& sessionState, const BattleTimelineConfig& timelineConfig)
{
	timelineState.emplace();
	BattleTimelineState& state = *timelineState;

	state.clockState       = TimelineClockState::Running;
	state.elapsedTime      = 0.f;
	state.config           = timelineConfig;
	state.leader.curLeader = sessionState.leaderEntity;

	state.alliesUsed  = 0;
	state.enemiesUsed = 0;
    
	// Allies
	for (int i = 0; i < sessionState.allies.memberCount; ++i)
	{
		const EntityID entity = sessionState.allies.members[i];
		if (entity == invalidEntity) continue;

		state.allies[i]             = {};
		state.allies[i].entity      = entity;
		state.allies[i].team        = BattleTeam::Ally;
		state.allies[i].ap          = TimelineAP{};
		state.allies[i].ATB         = TimelineGauge{ 0.f, state.config.gaugeMaxValue, state.config.gaugeFillSpeed };
		state.allies[i].gateState   = TimelineUnitGate::Open;
		state.allies[i].motionState = TimelineMotionState::Queued;
		state.allies[i].canAction   = state.config.canAction;

		state.alliesRuntime[i] = {};
		state.alliesRuntime[i].role.control    = (entity == state.leader.curLeader) ? TimelineControlType::Player : TimelineControlType::Ally;
		state.alliesRuntime[i].role.allowCombo = (state.alliesRuntime[i].role.control == TimelineControlType::Player);
		FillSkillCatalog(entity, state.alliesRuntime[i].skillCatalog);
		idxByEntity[entity] = { BattleTeam::Ally, i };
		++state.alliesUsed;
	}
	// Enemies
	for (int i = 0; i < sessionState.enemies.memberCount ; ++i)
	{
		const EntityID entity = sessionState.enemies.members[i];
		if (entity == invalidEntity) continue;

		state.enemies[i]             = {};
		state.enemies[i].entity      = entity;
		state.enemies[i].team        = BattleTeam::Enemy;
		state.enemies[i].ap          = TimelineAP{};
		state.enemies[i].ATB         = TimelineGauge{ 0.f, state.config.gaugeMaxValue, state.config.gaugeFillSpeed, false };
		state.enemies[i].gateState   = TimelineUnitGate::Open;
		state.enemies[i].motionState = TimelineMotionState::Queued;
		state.enemies[i].canAction   = state.config.canAction;

		state.enemiesRuntime[i] = {};
		state.enemiesRuntime[i].role.control = TimelineControlType::Enemy;
		state.enemiesRuntime[i].role.allowCombo = false;
		FillSkillCatalog(entity, state.enemiesRuntime[i].skillCatalog);
		idxByEntity[entity] = { BattleTeam::Enemy, i };
		++state.enemiesUsed;
	}
	eventQueues.clear();
}

void BattleTimelineSystem::Tick(float dt)
{
	if (!timelineState.has_value()) return;
	BattleTimelineState& state = *timelineState;

	if (state.clockState != TimelineClockState::Running) return;
	state.elapsedTime += dt;

	// Allies
	for (int i = 0; i < state.alliesUsed; ++i)
	{
		TimelineUnitState& unit = state.allies[i];
		if (unit.entity == invalidEntity) continue;
		AdvanceGauge(unit, dt, unit.entity, BattleTeam::Ally);
	}

	// Enemies
	for (int i = 0; i < state.enemiesUsed; ++i)
	{
		TimelineUnitState& unit = state.enemies[i];
		if (unit.entity == invalidEntity) continue;
		AdvanceGauge(unit, dt, unit.entity, BattleTeam::Enemy);
	}

	AutoCommitForAI();

	if (state.leader.curLeader != invalidEntity)
	{
		BattleTeam leaderTeam{};
		int        leaderSlot{};
		if (ResolveIdxByEntity(state.leader.curLeader, leaderTeam, leaderSlot))
		{
			assert(leaderTeam == BattleTeam::Ally && "Leader is expected to be Ally here");
			TimelineUnitState& unit = (leaderTeam == BattleTeam::Ally) ? state.allies[leaderSlot] : state.enemies[leaderSlot];
			TimelineUnitRunTime& run = (leaderTeam == BattleTeam::Ally) ? state.alliesRuntime[leaderSlot] : state.enemiesRuntime[leaderSlot];

			if (run.role.control == TimelineControlType::Player && IsUnitReadyToAct(unit.entity) && !run.inputQueue.Empty())
			{
				const TimelineActionIntent intent = run.inputQueue.pendingCombos.front();
				if (TryCommitIntent(unit.entity, intent))
				{
					auto& queue = run.inputQueue.pendingCombos;
					queue.erase(queue.begin());
				}
			}
		}
	}
}

void BattleTimelineSystem::EndSession()
{
	if (!timelineState.has_value()) return;
	timelineState.reset();
	idxByEntity.clear();
	eventQueues.clear();
}

bool BattleTimelineSystem::EnqueuePlayerIntent(EntityID playerEntity, const TimelineActionIntent& intent)
{
	if (!timelineState.has_value()) return false;

	BattleTeam team{};
	int slotIdx{};
	if (!ResolveIdxByEntity(playerEntity, team, slotIdx)) return false;

	BattleTimelineState& state = *timelineState;
	TimelineUnitRunTime& runTime = (team == BattleTeam::Ally) ? state.alliesRuntime[slotIdx] : state.enemiesRuntime[slotIdx];
	if (runTime.role.control != TimelineControlType::Player) return false;

	runTime.inputQueue.pendingCombos.push_back(intent);
	return true;
}

bool BattleTimelineSystem::TryCommitIntent(EntityID entity, const TimelineActionIntent& intent)
{
	if (!timelineState.has_value()) return false;

	BattleTeam team{};
	int slotIdx{};
	if (!ResolveIdxByEntity(entity, team, slotIdx)) return false;

	BattleTimelineState& state = *timelineState;
	TimelineUnitState&   unit  = (team == BattleTeam::Ally) ? state.allies[slotIdx]        : state.enemies[slotIdx];
	TimelineUnitRunTime& run   = (team == BattleTeam::Ally) ? state.alliesRuntime[slotIdx] : state.enemiesRuntime[slotIdx];

	return CommitInternal(unit, run, intent, entity, team);
}

void BattleTimelineSystem::AutoCommitForAI()
{
	if (!timelineState.has_value()) return;
	BattleTimelineState& state = *timelineState;

	// Allies
	for (int i = 0; i < state.alliesUsed; ++i)
	{
		TimelineUnitRunTime& run = state.alliesRuntime[i];
		if (run.role.control == TimelineControlType::Player) continue;

		TimelineUnitState& unit = state.allies[i];
		if (!IsUnitReadyToAct(unit.entity)) continue;

		TimelineActionIntent aiIntent{};
		if (BuildAiIntent(unit, run, aiIntent))
			CommitInternal(unit, run, aiIntent, unit.entity, BattleTeam::Ally);
	}
	// Enemies
	for (int i = 0; i < state.enemiesUsed; ++i)
	{
		TimelineUnitRunTime& run = state.enemiesRuntime[i];
		if (run.role.control != TimelineControlType::Enemy) continue;

		TimelineUnitState& unit = state.enemies[i];
		if (!IsUnitReadyToAct(unit.entity)) continue;

		TimelineActionIntent aiIntent{};
		if (BuildAiIntent(unit, run, aiIntent))
			CommitInternal(unit, run, aiIntent, unit.entity, BattleTeam::Enemy);
	}
}

void BattleTimelineSystem::NotifyActionFinished(EntityID entity, const TimelineActionIntent& finishedIntent)
{
	if (!timelineState.has_value()) return;

	BattleTeam team{};
	int slotIdx{};
	if (!ResolveIdxByEntity(entity, team, slotIdx)) return;

	BattleTimelineState& state = *timelineState;
	TimelineUnitState&   unit  = (team == BattleTeam::Ally) ? state.allies[slotIdx]        : state.enemies[slotIdx];
	TimelineUnitRunTime& run   = (team == BattleTeam::Ally) ? state.alliesRuntime[slotIdx] : state.enemiesRuntime[slotIdx];

	ApplyResolveReward(unit, run, finishedIntent, entity, team);
	unit.motionState = TimelineMotionState::Queued;
	unit.ATB.isFrozen = false;
	PushEvent(BattleTimelineEventType::ActionFinished, entity, team, 0);
}

bool BattleTimelineSystem::SwapLeader(EntityID newLeaderEntity)
{
	if (!timelineState.has_value()) return false;
	BattleTimelineState& state = *timelineState;

	if (!state.leader.swapApOnLeaderChange)
	{
		state.leader.curLeader = newLeaderEntity;
		return true;
	}

	BattleTeam curTeam{};
	int curSlot{};
	BattleTeam newTeam{};
	int newSlot{};

	if (state.leader.curLeader == invalidEntity)                       return false;
	if (!ResolveIdxByEntity(state.leader.curLeader, curTeam, curSlot)) return false;
	if (!ResolveIdxByEntity(newLeaderEntity, newTeam, newSlot))        return false;
	return true;
}

bool BattleTimelineSystem::TryGetUnitState(BattleTeam team, int slotIdx, const TimelineUnitState*& outState) const
{
	if (!timelineState.has_value()) return false;
	const BattleTimelineState& state = *timelineState;

	if (team == BattleTeam::Ally)
	{
		if (slotIdx >= state.alliesUsed) return false;
		outState = &state.allies[slotIdx];
		return true;
	}
	else if (team == BattleTeam::Enemy)
	{
		if (slotIdx >= state.enemiesUsed) return false;
		outState = &state.enemies[slotIdx];
		return true;
	}
	else
		return false;
}

bool BattleTimelineSystem::TryGetUnitStateByEntity(EntityID entity, BattleTeam& outTeam, int& outSlotIdx, const TimelineUnitState*& outState) const
{
	if (!timelineState.has_value()) return false;
	auto it = idxByEntity.find(entity);
	if (it == idxByEntity.end()) return false;

	outTeam    = it->second.first;
	outSlotIdx = it->second.second;

	const BattleTimelineState& state                = *timelineState;
	if (outTeam == BattleTeam::Ally)       outState = &state.allies[outSlotIdx];
	else if (outTeam == BattleTeam::Enemy) outState = &state.enemies[outSlotIdx];
	else                                   return false;
	return true;
}

bool BattleTimelineSystem::IsGaugeFull(EntityID entity) const
{
	const TimelineUnitState* unit{};
	BattleTeam team{};
	int slotIdx{};
	if (!TryGetUnitStateByEntity(entity, team, slotIdx, unit)) return false;
	return (unit->ATB.curValue >= unit->ATB.maxValue);
}

bool BattleTimelineSystem::IsUnitReadyToAct(EntityID entity) const
{
	const TimelineUnitState* unit{};
	BattleTeam team{};
	int slotIndex{};
	if (!TryGetUnitStateByEntity(entity, team, slotIndex, unit)) return false;

	if (unit->gateState != TimelineUnitGate::Open)        return false;
	if (!unit->canAction)                                 return false;
	if (unit->motionState != TimelineMotionState::Queued) return false;
	return (unit->ATB.curValue >= unit->ATB.maxValue);
}

void BattleTimelineSystem::SetClock(TimelineClockState newState)
{
	if (!timelineState.has_value()) return;
	BattleTimelineState& state = *timelineState;

	if (state.clockState == newState) return;
	state.clockState = newState;

	PushEvent((newState == TimelineClockState::Stopped) ? BattleTimelineEventType::TimelinePaused 
		                                                : BattleTimelineEventType::TimelineResumed, invalidEntity, BattleTeam::Ally, 0);
}

void BattleTimelineSystem::SetUnitGate(EntityID entity, TimelineUnitGate gate)
{
	if (!timelineState.has_value()) return;

	BattleTeam team{};
	int slotIdx{};
	if (!ResolveIdxByEntity(entity, team, slotIdx)) return;

	BattleTimelineState& state = *timelineState;
	TimelineUnitState&   unit  = (team == BattleTeam::Ally) ? state.allies[slotIdx] : state.enemies[slotIdx];
	unit.gateState = gate;
}

void BattleTimelineSystem::SetUnitCanAction(EntityID entity, bool canAction)
{
	if (!timelineState.has_value()) return;

	BattleTeam team{};
	int slotIdx{};
	if (!ResolveIdxByEntity(entity, team, slotIdx)) return;

	BattleTimelineState& state = *timelineState;
	TimelineUnitState&   unit  = (team == BattleTeam::Ally) ? state.allies[slotIdx] : state.enemies[slotIdx];
	unit.canAction = canAction;
}

bool BattleTimelineSystem::ResolveIdxByEntity(EntityID entity, BattleTeam& outTeam, int& outSlotIdx)
{
	auto it = idxByEntity.find(entity);
	if (it == idxByEntity.end()) return false;
	outTeam    = it->second.first;
	outSlotIdx = it->second.second;
	return true;
}

void BattleTimelineSystem::PushEvent(BattleTimelineEventType type, EntityID subject, BattleTeam team, int deltaAp)
{
	BattleTimelineEvent event{};
	event.eventType     = type;
	event.subjectEntity = subject;
	event.subjectTeam   = team;
	event.deltaAp       = deltaAp;
	eventQueues.push_back(event);
}

void BattleTimelineSystem::AdvanceGauge(TimelineUnitState& unit, float dt, EntityID entity, BattleTeam team)
{
	if (unit.ATB.isFrozen) return;
	if (unit.gateState != TimelineUnitGate::Open) return;
	if (!unit.canAction) return;
	if (unit.motionState == TimelineMotionState::Executing) return;

	const float prevValue = unit.ATB.curValue;
	unit.ATB.curValue += unit.ATB.fillSpeed * dt;
	if (unit.ATB.curValue > unit.ATB.maxValue) 
		unit.ATB.curValue = unit.ATB.maxValue;

	if (prevValue < unit.ATB.maxValue && unit.ATB.curValue >= unit.ATB.maxValue)
		PushEvent(BattleTimelineEventType::FullGauge, entity, team, 0);
}

bool BattleTimelineSystem::CommitInternal(TimelineUnitState& unitState, TimelineUnitRunTime& unitRunTime, TimelineActionIntent intent, EntityID entity, BattleTeam team)
{
	if (!unitState.canAction)                                 return false;
	if (unitState.gateState    != TimelineUnitGate::Open)     return false;
	if (unitState.motionState != TimelineMotionState::Queued) return false;
	if (unitState.ATB.curValue < unitState.ATB.maxValue)      return false;

	const int resolvedCost = ResolveSkillApCost(entity, intent.specialTag);
	intent.apCost = resolvedCost;

	if (intent.apCost > unitState.ap.curAp) return false;

	unitState.ap.curAp -= intent.apCost;
	if (unitState.ap.curAp < 0) 
		unitState.ap.curAp = 0;

	unitState.ATB.curValue  = 0.f;
	unitState.ATB.isFrozen  = true;
	unitState.pendingIntent = intent;
	unitState.activeIntent  = intent;
	unitState.motionState   = TimelineMotionState::Preparing;

	PushEvent(BattleTimelineEventType::ActionCommitted, entity, team, -intent.apCost);

	unitState.motionState = TimelineMotionState::Executing;
	return true;
}

bool BattleTimelineSystem::BuildAiIntent(TimelineUnitState& unitState,TimelineUnitRunTime& unitRuntime, TimelineActionIntent& outIntent)
{
	const EntityID targetEntity = ResolveOpponentTargetEntity(unitState.team);
	if (targetEntity == invalidEntity) return false;

	vector<const TimelineSkillInfo*> affordable;
	affordable.reserve(unitRuntime.skillCatalog.size());
	for (const TimelineSkillInfo& info : unitRuntime.skillCatalog)
	{
		if (info.apCost <= unitState.ap.curAp)
			affordable.push_back(&info);
	}

	const TimelineUnitPolicy& policy = unitRuntime.policy;
	const bool preferCostlyNow = (policy.aiPreferSkillAtMaxAp && unitState.ap.curAp >= unitState.ap.maxAp);

	const TimelineSkillInfo* chosen = nullptr;

	if (preferCostlyNow)
	{
		vector<const TimelineSkillInfo*> costly;
		costly.reserve(affordable.size());
		for (const TimelineSkillInfo* p : affordable)
			if (p->apCost > 0) costly.push_back(p);

		if (!costly.empty())
		{
			if (policy.aiRandomAmongAfforable)
			{
				static mt19937 rng{ random_device{}() };
				uniform_int_distribution<int> dist(0, static_cast<int>(costly.size() - 1));
				chosen = costly[dist(rng)];
			}
			else
				chosen = costly.front();
		}
	}

	if (!chosen)
	{
		for (const TimelineSkillInfo* p : affordable)
			if (p->apCost == 0) { chosen = p; break; }

		if (!chosen && !affordable.empty())
		{
			if (policy.aiRandomAmongAfforable)
			{
				static mt19937 rng{ random_device{}() };
				uniform_int_distribution<int> dist(0, static_cast<int>(affordable.size() - 1));
				chosen = affordable[dist(rng)];
			}
			else
				chosen = affordable.front();
		}
	}

	if (!chosen) return false;

	outIntent = {};
	outIntent.targetEntity = targetEntity;
	outIntent.battleCmd    = (chosen->apCost == 0) ? BattleCommand::AttackBasic : BattleCommand::Skill; 
	outIntent.specialTag   = chosen->tag;  
	outIntent.apCost       = chosen->apCost;
	return true;
}

void BattleTimelineSystem::ApplyApDelta(TimelineUnitState& unit, EntityID entity, BattleTeam team, int deltaAp)
{
	const int before = unit.ap.curAp;
	unit.ap.curAp   += deltaAp;
	if (unit.ap.curAp < 0) 
		unit.ap.curAp = 0;
	if (unit.ap.curAp > unit.ap.maxAp)
		unit.ap.curAp = unit.ap.maxAp;

	const int applied = unit.ap.curAp - before;
	if (applied != 0) 
		PushEvent(BattleTimelineEventType::ApChanged, entity, team, applied);
}

void BattleTimelineSystem::ApplyResolveReward(TimelineUnitState& unitState, const TimelineUnitRunTime& unitRuntime, const TimelineActionIntent& resolvedIntent, EntityID entity, BattleTeam team)
{
	const bool isBasic = (resolvedIntent.specialTag.has_value() &&
		resolvedIntent.specialTag.value() == SpecialAnimTag::BasicAttack);

	const int  deltaAp = isBasic ? unitRuntime.policy.apGainBasicAttack
		: unitRuntime.policy.apGainSkillAttack;

	ApplyApDelta(unitState, entity, team, deltaAp);
}

int BattleTimelineSystem::ResolveSkillApCost(EntityID entity, const optional<SpecialAnimTag>& specialTag) const
{
	if (!specialTag.has_value()) return 0;

	auto& actionReg = registry.Get<ActionAnimRegistry>();
	auto& dataSys   = registry.Get<CharacterDataSystem>();

	const CharacterID ch = dataSys.GetCharacterID(entity);
	const ActionAnimSpec* spec = actionReg.TryGet(ch);
	if (!spec) return 0;

	auto it = spec->apCostByTag.find(specialTag.value());
	return (it == spec->apCostByTag.end()) ? 0 : it->second;
}

void BattleTimelineSystem::FillSkillCatalog(EntityID entity, vector<TimelineSkillInfo>& outCatalog) const
{
	outCatalog.clear();

	auto* actionReg = registry.TryGet<ActionAnimRegistry>();
	auto* dataSys = registry.TryGet<CharacterDataSystem>();
	if (!actionReg || !dataSys) return;

	const CharacterID characterId = dataSys->GetCharacterID(entity);
	const ActionAnimSpec*    spec = actionReg->TryGet(characterId);
	if (!spec) return;

	auto shouldExpose = [](SpecialAnimTag tag)
		{
			if (tag == SpecialAnimTag::Intro) return false; 
			return true;
		};

	for (const auto& pair : spec->specials)
	{
		const SpecialAnimTag tag = pair.first;
		if (!shouldExpose(tag)) continue;

		int apCost = 0;
		if (auto it = spec->apCostByTag.find(tag); it != spec->apCostByTag.end())
			apCost = it->second;

		outCatalog.push_back(TimelineSkillInfo{ tag, apCost });
	}
}

EntityID BattleTimelineSystem::ResolveOpponentTargetEntity(BattleTeam myTeam) const
{
	if (!timelineState.has_value()) return invalidEntity;
	const BattleTimelineState& state = *timelineState;

	const BattleTeam opponentTeam = OppositeTeam(myTeam);
	if (opponentTeam == BattleTeam::Ally)
		return (state.alliesUsed > 0) ? state.allies[0].entity : invalidEntity;
	else
		return (state.enemiesUsed > 0) ? state.enemies[0].entity : invalidEntity;
}