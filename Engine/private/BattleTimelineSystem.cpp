#include "Enginepch.h"

static constexpr int kMaxCount = 3;
static inline BattleTeam OppositeTeam(BattleTeam team)
{
	assert(team != BattleTeam::Neutral && "OppositeTeam: Neutral is not allowed");
	return (team == BattleTeam::Ally) ? BattleTeam::Enemy : BattleTeam::Ally;
}
// ---------------------------------------------------------------------------------------------------------------------------
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
		state.alliesRuntime[i].role.control = (entity == state.leader.curLeader) ? TimelineControlType::Player : TimelineControlType::Ally;
		state.alliesRuntime[i].role.allowCombo = (state.alliesRuntime[i].role.control == TimelineControlType::Player);
		state.alliesRuntime[i].skillCatalog.push_back(TimelineSkillInfo{ L"basic", 0 });
		state.alliesRuntime[i].skillCatalog.push_back(TimelineSkillInfo{ L"skillA", 3 });

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
		state.enemiesRuntime[i].skillCatalog.push_back(TimelineSkillInfo{ L"basic", 0 });
		state.enemiesRuntime[i].skillCatalog.push_back(TimelineSkillInfo{ L"skillA", 3 });

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
	assert(team == BattleTeam::Ally && "EnqueuePlayerIntent: only allies can be player-controlled");

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
	PushEvent(BattleTimelineEventType::ActionFinished, entity, team, 0, L"execution finished");
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
	{
		assert(false && "TryGetUnitState: Neutral is invalid here");
		return false;
	}
}

bool BattleTimelineSystem::TryGetUnitStateByEntity(EntityID entity, BattleTeam& outTeam, int& outSlotIdx, const TimelineUnitState*& outState) const
{
	if (!timelineState.has_value()) return false;
	auto it = idxByEntity.find(entity);
	if (it == idxByEntity.end()) return false;

	outTeam    = it->second.first;
	outSlotIdx = it->second.second;

	const BattleTimelineState& state = *timelineState;
	if (outTeam == BattleTeam::Ally)
		outState = &state.allies[outSlotIdx];
	else if (outTeam == BattleTeam::Enemy)
		outState = &state.enemies[outSlotIdx];
	else
	{
		assert(false && "TryGetUnitStateEntity: Neutral is invalid");
		return false;
	}
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

	if (newState == TimelineClockState::Stopped)
		PushEvent(BattleTimelineEventType::TimelinePaused, invalidEntity, BattleTeam::Ally, 0, L"timeline paused");
	else
		PushEvent(BattleTimelineEventType::TimelineResumed, invalidEntity, BattleTeam::Ally, 0, L"timeline resumed");
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

void BattleTimelineSystem::PushEvent(BattleTimelineEventType type, EntityID subject, BattleTeam team, int deltaAp, const wstring& note)
{
	BattleTimelineEvent event{};
	event.eventType     = type;
	event.subjectEntity = subject;
	event.subjectTeam   = team;
	event.deltaAp       = deltaAp;
	event.note          = note;
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
	if (unit.ATB.curValue > unit.ATB.maxValue) unit.ATB.curValue = unit.ATB.maxValue;

	if (prevValue < unit.ATB.maxValue && unit.ATB.curValue >= unit.ATB.maxValue)
		PushEvent(BattleTimelineEventType::FullGauge, entity, team, 0, L"gauge full");
}

bool BattleTimelineSystem::CommitInternal(TimelineUnitState& unitState, TimelineUnitRunTime& unitRunTime, const TimelineActionIntent& intent, EntityID entity, BattleTeam team)
{
	if (!unitState.canAction)                                 return false;
	if (unitState.gateState != TimelineUnitGate::Open)        return false;
	if (unitState.motionState != TimelineMotionState::Queued) return false;
	if (unitState.ATB.curValue < unitState.ATB.maxValue)      return false;

	const int apCost = intent.apCost;
	if (apCost > unitState.ap.curAp) return false;

	unitState.ap.curAp -= apCost;
	if (unitState.ap.curAp < 0)
		unitState.ap.curAp = 0;

	unitState.ATB.curValue = 0.f;
	unitState.ATB.isFrozen = true;

	unitState.pendingIntent = intent;   
	unitState.activeIntent  = intent;
	unitState.motionState   = TimelineMotionState::Preparing;

	PushEvent(BattleTimelineEventType::ActionCommitted, entity, team, -apCost, L"commit intent");
	unitState.motionState = TimelineMotionState::Executing;
	return true;
}

bool BattleTimelineSystem::BuildAiIntent(const TimelineUnitState& unitState, const TimelineUnitRunTime& unitRuntime, TimelineActionIntent& outIntent)
{
	assert(unitState.ATB.curValue >= unitState.ATB.maxValue);

	BattleTeam opponent = OppositeTeam(unitState.team);
	EntityID targetEntity = invalidEntity;
	if (timelineState)
	{
		const BattleTimelineState& state = *timelineState;
		if (opponent == BattleTeam::Ally)
		{
			if (state.alliesUsed > 0)
				targetEntity = state.allies[0].entity;
		}
		else
		{
			if (state.enemiesUsed > 0)
				targetEntity = state.enemies[0].entity;
		}
	}
	if (targetEntity == invalidEntity) return false;

	const TimelineUnitPolicy& policy = unitRuntime.policy;

	vector<const TimelineSkillInfo*> afforable;
	afforable.reserve(unitRuntime.skillCatalog.size());
	for (const auto& skill : unitRuntime.skillCatalog)
	{
		if (skill.apCost <= unitState.ap.curAp)
			afforable.push_back(&skill);
	}

	const TimelineSkillInfo* chosen{};
	if (policy.aiPreferSkillAtMaxAp && unitState.ap.curAp >= unitState.ap.maxAp)
	{
		vector<const TimelineSkillInfo*> costly;
		for (auto* ptr : afforable)
		{
			if (ptr->apCost > 0)
				costly.push_back(ptr);
		}

		if (!costly.empty())
			chosen = policy.aiRandomAmongAfforable ? costly[rand() % costly.size()] : costly.front();
	}
	if (!chosen)
	{
		for (auto* ptr : afforable)
		{
			if (ptr->apCost == 0)
			{
				chosen = ptr;
				break;
			}
		}
		if (!chosen && !afforable.empty())
			chosen = afforable.front();
	}
	if (!chosen) return false;

	// Intent
	outIntent              = {};
	outIntent.targetEntity = targetEntity;
	outIntent.apCost       = chosen->apCost;
	if (chosen->apCost == 0)
	{
		outIntent.battleCmd = BattleCommand::AttackBasic;
		outIntent.skillKey = L"basic";
	}
	else
	{
		outIntent.battleCmd = BattleCommand::Skill;
		outIntent.skillKey = chosen->skillKey;
	}
	return true;
}

void BattleTimelineSystem::ApplyApDelta(TimelineUnitState& unit, EntityID entity, BattleTeam team, int deltaAp, const wstring& note)
{
	const int before = unit.ap.curAp;
	unit.ap.curAp += deltaAp;
	if (unit.ap.curAp < 0) unit.ap.curAp = 0;
	if (unit.ap.curAp > unit.ap.maxAp)
		unit.ap.curAp = unit.ap.maxAp;

	const int applied = unit.ap.curAp - before;
	if (applied != 0)
		PushEvent(BattleTimelineEventType::ApChanged, entity, team, applied, note);
}

void BattleTimelineSystem::ApplyResolveReward(TimelineUnitState& unitState, const TimelineUnitRunTime& unitRuntime, const TimelineActionIntent& resolvedIntent, EntityID entity, BattleTeam team)
{
	const int deltaAp =(resolvedIntent.battleCmd == BattleCommand::AttackBasic) ? unitRuntime.policy.apGainBasicAttack : unitRuntime.policy.apGainSkillAttack;
	ApplyApDelta(unitState, entity, team, deltaAp, (resolvedIntent.battleCmd == BattleCommand::AttackBasic) ? L"reward basic" : L"reward skill");
}