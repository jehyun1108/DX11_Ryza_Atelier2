#include "Enginepch.h"

static inline float Lerp01(float a, float b, float t)
{
	return a + (b - a) * t;
}
// ===============================================================================================================
void BattleTimelinePresenter::OnBoot()
{
	eventBus    = &registry.Get<BattleEventBus>();
	uiRegistry  = &registry.Get<UIRegistry>();
	uiAnimSys   = &registry.Get<UIAnimSystem>();
	timelineSys = &registry.Get<BattleTimelineSystem>();
	dataSys     = &registry.Get<CharacterDataSystem>();

	assert(eventBus && uiRegistry && uiAnimSys && timelineSys);
}

void BattleTimelinePresenter::Enter()
{
	EnsureConfigured();
	const auto* state = timelineSys->TryGetState();
	if (!state) return;

	for (int i = 0; i < state->alliesUsed; ++i) 
	{
		const EntityID entity = state->allies[i].entity;
		if (!entity) continue;
		if (!EnsureIconInstance(entity, BattleTeam::Ally, i)) continue;

		const wstring key = BuildInstanceKey(entity);
		const float x = alliesLayout.xStart;
		const float y = alliesLayout.yBase;
		uiRegistry->SetLocalPos(key, x, y);

		const bool isLeader = (entity == timelineSys->GetLeader());
		const int z = ComposeZOrder(BattleTeam::Ally, isLeader, 0.f, i);
		uiRegistry->SetZOrder(key, z);

		uiAnimSys->SetScale(key, config.defaultIconScale, config.defaultIconScale);
	}

	for (int i = 0; i < state->enemiesUsed; ++i)
	{
		const EntityID entity = state->enemies[i].entity;
		if (!entity) continue;
		if (!EnsureIconInstance(entity, BattleTeam::Enemy, i)) continue;

		const wstring key = BuildInstanceKey(entity);
		const float x = enemiesLayout.xStart;
		const float y = enemiesLayout.yBase; 
		uiRegistry->SetLocalPos(key, x, y);

		const bool isLeader = (entity == timelineSys->GetLeader());
		const int z = ComposeZOrder(BattleTeam::Enemy, isLeader, 0.f, i);
		uiRegistry->SetZOrder(key, z);
		uiAnimSys->SetScale(key, config.defaultIconScale, config.defaultIconScale);
	}

	leaderEntity = timelineSys->GetLeader();
	ApplyLeaderHighlight(); 
	WireEventSubscriptions();
}

void BattleTimelinePresenter::Tick(float dt)
{
	const auto* state = timelineSys->TryGetState();
	if (!state) return;

	for (int i = 0; i < state->alliesUsed; ++i)
	{
		const auto& unit = state->allies[i];
		const EntityID entity = unit.entity;
		if (entity == 0) continue;

		if (!EnsureIconInstance(entity, BattleTeam::Ally, i)) continue;
		const wstring key = BuildInstanceKey(entity);

		const float x = ResolveXPos(unit, false);
		const float y = alliesLayout.yBase;
		uiRegistry->SetLocalPos(key, x, y);

		const float ratio = (unit.ATB.maxValue > 0.f) ? min(unit.ATB.curValue / unit.ATB.maxValue, 1.f) : 0.f;
		const float isLeader = (entity == timelineSys->GetLeader());
		const int z = ComposeZOrder(BattleTeam::Ally, isLeader, ratio, i);
		uiRegistry->SetZOrder(key, z);
	}

	for (int i = 0; i < state->enemiesUsed; ++i)
	{
		const auto& unit = state->enemies[i];
		const EntityID entity = unit.entity;
		if (entity == 0) continue;

		if (!EnsureIconInstance(entity, BattleTeam::Enemy, i)) continue;
		const wstring key = BuildInstanceKey(entity);

		const float x = ResolveXPos(unit, true);
		const float y = enemiesLayout.yBase;
		uiRegistry->SetLocalPos(key, x, y);

		const float ratio = (unit.ATB.maxValue > 0.f) ? min(unit.ATB.curValue / unit.ATB.maxValue, 1.f) : 0.f;
		const bool isLeader = (entity == timelineSys->GetLeader());
		const int z = ComposeZOrder(BattleTeam::Enemy, isLeader, ratio, i);
		uiRegistry->SetZOrder(key, z);
	}
}

void BattleTimelinePresenter::Exit()
{
	UnWireEventSubscriptions();

	const auto* state = timelineSys->TryGetState();
	if (state)
	{
		for (int i = 0; i < state->alliesUsed; ++i)
		{
			const EntityID entity = state->allies[i].entity;
			if (!entity) continue;
			SetIconEnabled(entity, false);
		}
		for (int i = 0; i < state->enemiesUsed; ++i)
		{
			const EntityID entity = state->enemies[i].entity;
			if (!entity) continue;
			SetIconEnabled(entity, false);
		}
	}
	leaderEntity = 0;
}

void BattleTimelinePresenter::SetAbsoluteLayout(float allyStartX, float allyReadyX, float enemyStartX, float enemyReadyX, float baseY)
{
	alliesLayout.xStart = allyStartX;
	alliesLayout.xReady = allyReadyX;
	alliesLayout.yBase = baseY;

	enemiesLayout.xStart = enemyStartX;
	enemiesLayout.xReady = enemyReadyX;
	enemiesLayout.yBase = baseY;
}

void BattleTimelinePresenter::EnsureConfigured()
{
	if (configured) return;

	ComputeLayoutsFromViewport();
	configured = true;
}

void BattleTimelinePresenter::ComputeLayoutsFromViewport()
{
	const auto& vp = GAME.GetViewport();           
	const float width = static_cast<float>(vp.Width);
	const float center = width * 0.5f;

	alliesLayout.xStart = config.marginLeft;
	alliesLayout.xReady = center - max(0.f, config.readyCenterBias);
	alliesLayout.yBase = config.laneYBase;

	enemiesLayout.xStart = width - config.marginRight;
	enemiesLayout.xReady = center + max(0.f, config.readyCenterBias);
	enemiesLayout.yBase = config.laneYBase;
}

float BattleTimelinePresenter::ResolveXPos(const TimelineUnitState& unitState, bool isEnemy) const
{
	if (unitState.motionState == TimelineMotionState::Executing)
		return isEnemy ? enemiesLayout.xReady : alliesLayout.xReady;

	const float ratio = (unitState.ATB.maxValue > 0.f) ? min(unitState.ATB.curValue / unitState.ATB.maxValue, 1.f) : 0.f;

	return isEnemy ? Lerp01(enemiesLayout.xStart, enemiesLayout.xReady, ratio) : Lerp01(alliesLayout.xStart, alliesLayout.xReady, ratio);
}

int BattleTimelinePresenter::ComputeZBias(bool isLeader, float progress01, int tieBreaker) const
{
	int bias = 0;

	if (isLeader)
		bias += config.zBiasLeaderBonus;

	const float t = clamp(progress01, 0.f, 1.f);
	bias += static_cast<int>(lround(t * static_cast<float>(config.zBiasProgressScale)));

	bias += tieBreaker * config.zBiasTieBreakerStep;
	return bias;
}

int BattleTimelinePresenter::ComposeZOrder(BattleTeam team, bool isLeader, float progress01, int tieBreaker) const
{
	const int base = (team == BattleTeam::Enemy) ? config.zBaseEnemies : config.zBaseAllies;
	return base + ComputeZBias(isLeader, progress01, tieBreaker);
}

void BattleTimelinePresenter::ApplyLeaderHighlight()
{
	if (leaderEntity == invalidEntity) return;
	const wstring key = BuildInstanceKey(leaderEntity);
	uiAnimSys->ScaleTo(key, config.leaderIconScale, config.leaderIconScale, config.scaleAnimInDur, UIEasing::EaseOutCubic);
}

void BattleTimelinePresenter::ClearLeaderHighlight()
{
	if (leaderEntity == invalidEntity) return;
	const wstring key = BuildInstanceKey(leaderEntity);
	uiAnimSys->ScaleTo(key, config.defaultIconScale, config.defaultIconScale, config.scaleAnimOutDur , UIEasing::EaseOutCubic);
}

bool BattleTimelinePresenter::EnsureIconInstance(EntityID entity, BattleTeam team, int slotIdx)
{
	(void)team; (void)slotIdx;
	const wstring* texKey = ResolveIconKey(entity);
	if (!texKey) return false;

	const wstring instKey = BuildInstanceKey(entity);

	if (uiRegistry->GetArchetypes().find(instKey) == uiRegistry->GetArchetypes().end())
	{
		UIArchetypeSpec spec{};
		spec.texKey = *texKey;
		spec.context = UIContext::Battle;
		spec.startEnabled = true;
		uiRegistry->RegisterArchetype(instKey, spec);
	}
	uiRegistry->Ensure(instKey);
	uiRegistry->SetEnabled(instKey, true);
	return true;
}

void BattleTimelinePresenter::SetIconEnabled(EntityID entity, bool enabled)
{
	const wstring key = BuildInstanceKey(entity);
	uiRegistry->SetEnabled(key, enabled);
}

wstring BattleTimelinePresenter::BuildInstanceKey(EntityID entity) const
{
	return L"timeline_icon_" + to_wstring(static_cast<_ulong>(entity));
}

const wstring* BattleTimelinePresenter::ResolveIconKey(EntityID entity) const
{
	return dataSys->TryGetTextureKey(entity, UITextureSlot::TimelineIcon);
}

void BattleTimelinePresenter::WireEventSubscriptions()
{
	if (subscriptionsWired) return;

	listenersId.push_back(eventBus->Subscribe(BattleBusEventType::TimelineFullGauge, [&](const BattleEvent& e)
		{
			OnFullGauge(e.subjectEntity);
		}));
	listenersId.push_back(eventBus->Subscribe(BattleBusEventType::TimelineActionCommitted, [&](const BattleEvent& e)
		{
			OnActionCommitted(e.subjectEntity);
		}));
	listenersId.push_back(eventBus->Subscribe(BattleBusEventType::TimelineActionFinished, [&](const BattleEvent& e)
		{
			OnActionFinished(e.subjectEntity);
		}));
	listenersId.push_back(eventBus->Subscribe(BattleBusEventType::LeaderChanged, [&](const BattleEvent& e)
		{
			OnLeaderChanged(e.subjectEntity);
		}));
	listenersId.push_back(eventBus->Subscribe(BattleBusEventType::TimelineApChanged, [&](const BattleEvent& e)
		{
			if (auto payload = get_if<EventPayload_ApChanged>(&e.payload))
				OnApChanged(e.subjectEntity, payload->deltaAp, payload->curAp, payload->maxAp);
		}));

	subscriptionsWired = true;
}

void BattleTimelinePresenter::UnWireEventSubscriptions()
{
	if (!subscriptionsWired) return;
	for (auto id : listenersId) 
		eventBus->Unsubscribe(id);
	listenersId.clear();
	subscriptionsWired = false;
}

void BattleTimelinePresenter::OnActionFinished(EntityID entity)
{
	BattleTeam team{};
	int slotIdx{};
	const TimelineUnitState* unit{};
	if (!timelineSys->TryGetUnitStateByEntity(entity, team, slotIdx, unit) || !unit) return;
	if (!EnsureIconInstance(entity, team, slotIdx)) return;

	const wstring key = BuildInstanceKey(entity);
	const float y = (team == BattleTeam::Enemy) ? enemiesLayout.yBase + static_cast<float>(slotIdx)
		                                        : alliesLayout.yBase + static_cast<float>(slotIdx);
	const float x = (team == BattleTeam::Enemy) ? enemiesLayout.xStart : alliesLayout.xStart;
	uiRegistry->SetLocalPos(key, x, y);
}

void BattleTimelinePresenter::OnLeaderChanged(EntityID newLeaderEntity)
{
	if (leaderEntity != invalidEntity && leaderEntity != newLeaderEntity)
	{
		const wstring prevKey = BuildInstanceKey(leaderEntity);
		uiAnimSys->ScaleTo(prevKey, config.defaultIconScale, config.defaultIconScale, config.scaleAnimOutDur, UIEasing::EaseOutCubic);
	}
	leaderEntity = newLeaderEntity;
	ApplyLeaderHighlight();
}