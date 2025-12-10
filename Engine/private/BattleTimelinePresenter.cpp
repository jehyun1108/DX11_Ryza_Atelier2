#include "Enginepch.h"
#include "BattleTimelinePresenter.h"
#include "PlayerInputPresenter.h"
#include "BattleEventBus.h"
#include "BattleAttributeSystem.h"

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
	targetSys   = &registry.Get<BattleTargetSystem>();
	attrSys     = &registry.Get<BattleAttributeSystem>();
}

void BattleTimelinePresenter::Enter()
{
	EnsureConfigured();
	auto state = timelineSys->GetState();

	for (int i = 0; i < state.alliesUsed; ++i) 
	{
		const EntityID entity = state.allies[i].entity;
		const wstring key = BuildInstanceKey(entity);
		const bool ok = EnsureIconInstance(entity, BattleTeam::Ally, i);

		uiRegistry->SetLocalPos(key, alliesLayout.xStart, alliesLayout.yBase);
		const int z = ComposeZOrder(BattleTeam::Ally, entity == timelineSys->GetLeader(), 0.f, i);
		uiRegistry->SetZOrder(key, z);
		uiAnimSys->SetScale(key, config.defaultIconScale, config.defaultIconScale);
	}

	for (int i = 0; i < state.enemiesUsed; ++i)
	{
		const EntityID entity = state.enemies[i].entity;
		const wstring key = BuildInstanceKey(entity);
		const bool ok = EnsureIconInstance(entity, BattleTeam::Enemy, i);

		uiRegistry->SetLocalPos(key, enemiesLayout.xStart, enemiesLayout.yBase);
		const int z = ComposeZOrder(BattleTeam::Enemy, entity == timelineSys->GetLeader(), 0.f, i);
		uiRegistry->SetZOrder(key, z);
		uiAnimSys->SetScale(key, config.defaultIconScale, config.defaultIconScale);
	}
	WireEventSubscriptions();

	leaderEntity = timelineSys->GetLeader();

	ApplyLeaderHighlight();
	UpdateTargetHighlight();

	EnsureWaitBar();
	UpdateWaitBar();

	uiRegistry->Ensure(leaderHighlightKey);
	uiRegistry->SetEnabled(leaderHighlightKey, true);
	uiAnimSys->SetScale(leaderHighlightKey, 1.f, 1.f);
	uiAnimSys->SetOpacity(leaderHighlightKey, 1.f);
	leaderHighlightT = 0.f;
	UpdateLeaderHighlightFX(0.f);
}

void BattleTimelinePresenter::Tick(float dt)
{
	auto state = timelineSys->GetState();

	const EntityID curLeader = timelineSys->GetLeader();
	OnLeaderChanged(curLeader);

	for (int i = 0; i < state.alliesUsed; ++i)
	{
		const auto& unit = state.allies[i];
		const EntityID entity = unit.entity;
		const wstring key = BuildInstanceKey(entity);
		const float x = ResolveXPos(unit, false);
		uiRegistry->SetLocalPos(key, x, alliesLayout.yBase);

		const float ratio = (unit.ATB.maxValue > 0.f) ? min(unit.ATB.curValue / unit.ATB.maxValue, 1.f) : 0.f;
		const int z = ComposeZOrder(BattleTeam::Ally, entity == curLeader, ratio, i);
		uiRegistry->SetZOrder(key, z);

		float hpRatio = attrSys->GetHpRatio01(entity);
		float alpha = (hpRatio > 0.f) ? 1.f : 0.f;
		uiAnimSys->SetOpacity(key, alpha);
	}

	for (int i = 0; i < state.enemiesUsed; ++i)
	{
		const auto& unit = state.enemies[i];
		const EntityID entity = unit.entity;
		const wstring key = BuildInstanceKey(entity);
		const float x = ResolveXPos(unit, true);
		uiRegistry->SetLocalPos(key, x, enemiesLayout.yBase);

		const float ratio = (unit.ATB.maxValue > 0.f) ? min(unit.ATB.curValue / unit.ATB.maxValue, 1.f) : 0.f;
		const int z = ComposeZOrder(BattleTeam::Enemy, entity == curLeader, ratio, i);
		uiRegistry->SetZOrder(key, z);

		 float hpRatio = attrSys->GetHpRatio01(entity);
        float alpha   = (hpRatio > 0.f) ? 1.f : 0.f;
        uiAnimSys->SetOpacity(key, alpha);
	}

	UpdateTargetHighlight();
	UpdateWaitBar();
	UpdateLeaderHighlightFX(dt);
}

void BattleTimelinePresenter::Exit()
{
	UnWireEventSubscriptions();
	auto state = timelineSys->GetState();

	for (int i = 0; i < state.alliesUsed;  ++i) SetIconEnabled(state.allies[i].entity,  false);
	for (int i = 0; i < state.enemiesUsed; ++i) SetIconEnabled(state.enemies[i].entity, false);

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

bool BattleTimelinePresenter::ResolveUnitForEntity(EntityID entity, BattleTeam& outTeam, int& outSlotIdx, const TimelineUnitState*& outUnit) const
{
	const auto& state = timelineSys->GetState();

	for (int i = 0; i < state.alliesUsed; ++i)
	{
		const auto& u = state.allies[i];
		if (u.entity == entity)
		{
			outTeam = BattleTeam::Ally;
			outSlotIdx = i;
			outUnit = &u;
			return true;
		}
	}

	for (int i = 0; i < state.enemiesUsed; ++i)
	{
		const auto& u = state.enemies[i];
		if (u.entity == entity)
		{
			outTeam = BattleTeam::Enemy;
			outSlotIdx = i;
			outUnit = &u;
			return true;
		}
	}

	return false;
}

void BattleTimelinePresenter::EnsureConfigured()
{
	if (configured) return;
	ComputeLayoutsFromViewport();
	configured = true;
}

void BattleTimelinePresenter::ComputeLayoutsFromViewport()
{
	const auto& vp     = GAME.GetViewport();           
	const float width  = static_cast<float>(vp.Width);
	const float center = width * 0.5f;

	alliesLayout.xStart = config.marginLeft;
	alliesLayout.xReady = center - max(0.f, config.readyCenterBias);
	alliesLayout.yBase  = config.laneYBase;

	enemiesLayout.xStart = width - config.marginRight;
	enemiesLayout.xReady = center + max(0.f, config.readyCenterBias);
	enemiesLayout.yBase  = config.laneYBase;
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
	if (isLeader) bias += config.zBiasLeaderBonus;
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
	const wstring key = BuildInstanceKey(leaderEntity);
	uiAnimSys->ScaleTo(key, config.leaderIconScale, config.leaderIconScale, config.scaleAnimInDur);
}

void BattleTimelinePresenter::ClearLeaderHighlight()
{
	const wstring key = BuildInstanceKey(leaderEntity);
	uiAnimSys->ScaleTo(key, config.defaultIconScale, config.defaultIconScale, config.scaleAnimOutDur);
}

bool BattleTimelinePresenter::EnsureIconInstance(EntityID entity, BattleTeam team, int slotIdx)
{
	const wstring texKey = dataSys->GetTextureKey(entity, UITextureSlot::TimelineIcon);
	const wstring instKey = BuildInstanceKey(entity);

	if (uiRegistry->GetArchetypes().find(instKey) == uiRegistry->GetArchetypes().end())
	{
		UIArchetypeSpec spec{};
		spec.texKey = texKey;
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
	for (auto id : listenersId) eventBus->Unsubscribe(id);
	listenersId.clear();
	subscriptionsWired = false;
}

void BattleTimelinePresenter::OnActionFinished(EntityID entity)
{
	BattleTeam team{};
	int slotIdx{};
	auto unit = timelineSys->GetUnitStateByEntity(entity, team, slotIdx);

	const wstring key = BuildInstanceKey(entity);
	const float y = (team == BattleTeam::Enemy) ? enemiesLayout.yBase : alliesLayout.yBase;
	const float x = (team == BattleTeam::Enemy) ? enemiesLayout.xStart : alliesLayout.xStart;
	uiRegistry->SetLocalPos(key, x, y);
}

void BattleTimelinePresenter::OnLeaderChanged(EntityID newLeaderEntity)
{
	if (leaderEntity != newLeaderEntity && leaderEntity != invalidEntity)
		ClearLeaderHighlight();

	leaderEntity = newLeaderEntity;

	ApplyLeaderHighlight();
	UpdateTargetHighlight();
}

wstring BattleTimelinePresenter::BuildTargetRingKey(EntityID target) const
{
	return L"timeline_targetring_" + to_wstring(static_cast<_ulong>(target));
}

void BattleTimelinePresenter::EnsureTargetRing(EntityID target)
{
	const wstring instKey = BuildTargetRingKey(target);

	if (uiRegistry->GetArchetypes().find(instKey) == uiRegistry->GetArchetypes().end())
	{
		UIArchetypeSpec spec{};
		spec.texKey       = targetRectKey;
		spec.context      = UIContext::Battle;
		spec.startEnabled = false;
		spec.pivot        = UIPivot::MidCenter;
		spec.sizeMode     = UISizeMode::Ratio;
		uiRegistry->RegisterArchetype(instKey, spec);
	}
	uiRegistry->Ensure(instKey);
}

void BattleTimelinePresenter::EnableTargetRingAt(EntityID target, BattleTeam team, int slotIdx, const TimelineUnitState& unit)
{
	EnsureTargetRing(target);

	const wstring ringKey  = BuildTargetRingKey(target);
	const float   x        = ResolveXPos(unit, team == BattleTeam::Enemy);
	const float   y        = (team == BattleTeam::Enemy) ? enemiesLayout.yBase : alliesLayout.yBase;
	const bool    isLeader = (target == timelineSys->GetLeader());
	const float   ratio    = (unit.ATB.maxValue > 0.f) ? min(unit.ATB.curValue / unit.ATB.maxValue, 1.f) : 0.f;
	const int     z        = ComposeZOrder(team, isLeader, ratio, slotIdx) + 5;

	uiRegistry->SetZOrder(ringKey, z);
	uiRegistry->SetLocalPos(ringKey, x, y);
	uiAnimSys->SetScale(ringKey, 1.5f, 1.5f);
	uiAnimSys->Spin(ringKey, 180.f);
	uiRegistry->SetEnabled(ringKey, true);
}

void BattleTimelinePresenter::DisableTargetRing(EntityID target)
{
	const wstring ringKey = BuildTargetRingKey(target);
	uiRegistry->SetEnabled(ringKey, false);
	uiAnimSys->StopSpin(ringKey);
}

void BattleTimelinePresenter::UpdateTargetHighlight()
{
	const EntityID leader = timelineSys->GetLeader();

	if (leader == 0u)
	{
		if (highlightedTarget != 0u)
		{
			DisableTargetRing(highlightedTarget);
			highlightedTarget = 0u;
		}
		return;
	}

	const EntityID target = targetSys->Get(leader);

	if (target == 0u)
	{
		if (highlightedTarget != 0u)
		{
			DisableTargetRing(highlightedTarget);
			highlightedTarget = 0u;
		}
		return;
	}

	BattleTeam team{};
	int slot{};
	const TimelineUnitState* unit = nullptr;

	if (!ResolveUnitForEntity(target, team, slot, unit))
	{
		if (highlightedTarget != 0u)
		{
			DisableTargetRing(highlightedTarget);
			highlightedTarget = 0u;
		}
		return;
	}

	if (highlightedTarget != 0u && highlightedTarget != target)
		DisableTargetRing(highlightedTarget);

	EnableTargetRingAt(target, team, slot, *unit);
	highlightedTarget = target;
}

void BattleTimelinePresenter::EnsureWaitBar()
{
	uiRegistry->Ensure(waitBarBackKey1);
	uiRegistry->Ensure(waitBarBackKey2);
	uiRegistry->Ensure(waitBarFrontKey);
	uiRegistry->Ensure(waitBarFullKey);

	uiRegistry->SetEnabled(waitBarBackKey1, true);
	uiRegistry->SetEnabled(waitBarBackKey2, true);
	uiRegistry->SetEnabled(waitBarFrontKey, true);
	uiRegistry->SetEnabled(waitBarFullKey, false);
	uiAnimSys->SetOpacity(waitBarFullKey, 0.f);
}

void BattleTimelinePresenter::UpdateWaitBar()
{
	const EntityID leader = timelineSys->GetLeader();
	if (leader == invalidEntity) { HideWaitBar(); return; }

	BattleTeam team{}; int slot{};
	const auto& unit = timelineSys->GetUnitStateByEntity(leader, team, slot);

	uiRegistry->SetEnabled(waitBarBackKey1, true);
	uiRegistry->SetEnabled(waitBarBackKey2, true);
	uiRegistry->SetEnabled(waitBarFrontKey, true);

	float ratio = (unit.ATB.maxValue > 0.f) ? (unit.ATB.curValue / unit.ATB.maxValue) : 0.f;
	ratio = clamp(ratio, 0.f, 1.f);
	uiRegistry->SetFillRatioX(waitBarFrontKey, ratio);

	const int zBase = ComposeZOrder(team, true, ratio, slot);
	uiRegistry->SetZOrder(waitBarBackKey1, zBase - 2);
	uiRegistry->SetZOrder(waitBarBackKey2, zBase - 1);
	uiRegistry->SetZOrder(waitBarFrontKey, zBase);
	uiRegistry->SetZOrder(waitBarFullKey, zBase + 1);

	const bool ready = timelineSys->IsUnitReadyToAct(leader);
	const bool showFull = ready || (showFullWhenExecuting && unit.motionState == TimelineMotionState::Executing);

	uiRegistry->SetEnabled(waitBarFullKey, showFull);
	if (showFull)
	{
		const float t = timelineSys->GetState().elapsedTime;
		const float blink = 0.5f + 0.5f * sinf(t * XM_2PI * 2.0f);
		uiAnimSys->SetOpacity(waitBarFullKey, blink);
	}
	else
		uiAnimSys->SetOpacity(waitBarFullKey, 0.f);
}

void BattleTimelinePresenter::HideWaitBar()
{
	uiRegistry->SetEnabled(waitBarBackKey1, false);
	uiRegistry->SetEnabled(waitBarBackKey2, false);
	uiRegistry->SetEnabled(waitBarFrontKey, false);
	uiRegistry->SetEnabled(waitBarFullKey,  false);
}

void BattleTimelinePresenter::UpdateLeaderHighlightFX(float dt)
{
	const EntityID leader = timelineSys->GetLeader();
	if (leader == invalidEntity)
	{
		uiRegistry->SetEnabled(leaderHighlightKey, false);
		return;
	}

	BattleTeam team{};
	int slot{};
	const auto& unit = timelineSys->GetUnitStateByEntity(leader, team, slot);

	const bool isEnemy = (team == BattleTeam::Enemy);
	const float x = ResolveXPos(unit, isEnemy);
	const float y = isEnemy ? enemiesLayout.yBase : alliesLayout.yBase;

	uiRegistry->SetEnabled(leaderHighlightKey, true);
	uiRegistry->SetLocalPos(leaderHighlightKey, x, y);

	const float ratio = (unit.ATB.maxValue > 0.f) ? min(unit.ATB.curValue / unit.ATB.maxValue, 1.f) : 0.f;
	const int zIcon = ComposeZOrder(team, true, ratio, slot);
	uiRegistry->SetZOrder(leaderHighlightKey, zIcon - 1);

	leaderHighlightT += dt;

	const float period = 1.2f;
	float localT = fmodf(leaderHighlightT, period); 
	float norm = localT / period;           

	float scale = 1.0f + 0.2f * norm;
	float alpha = 1.0f - norm;

	uiAnimSys->SetScale(leaderHighlightKey, scale, scale);
	uiAnimSys->SetOpacity(leaderHighlightKey, alpha);
}