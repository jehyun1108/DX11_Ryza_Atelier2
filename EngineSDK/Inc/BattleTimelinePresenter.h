#pragma once

#include "TimelinePresenterData.h"

NS_BEGIN(Engine)
class BattleEventBus;

class BattleTimelinePresenter : public ISystem
{
public:
	explicit BattleTimelinePresenter(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void Enter();
	void Tick(float dt);
	void Exit();

	void SetAbsoluteLayout(float allyStartX, float allyReadyX, float enemyStartX, float enemyReadyX, float baseY);
	bool ResolveUnitForEntity(EntityID entity, BattleTeam& outTeam, int& outSlotIdx, const TimelineUnitState*& outUnit) const;

private:
	void  EnsureConfigured();
	void  ComputeLayoutsFromViewport();

	float ResolveXPos(const TimelineUnitState& unitState, bool isEnemy) const;
	int   ComputeZBias(bool isLeader, float progress01, int tieBreaker) const;
	int   ComposeZOrder(BattleTeam team, bool isLeader, float progress01, int tieBreaker) const;

	// 리더 연출
	void ApplyLeaderHighlight();
	void ClearLeaderHighlight();

	bool           EnsureIconInstance(EntityID entity, BattleTeam team, int slotIdx);
	void           SetIconEnabled(EntityID entity, bool enabled);
	wstring        BuildInstanceKey(EntityID entity) const;

	void    WireEventSubscriptions();
	void    UnWireEventSubscriptions();

	void    OnFullGauge(EntityID entity) {}
	void    OnActionCommitted(EntityID entity) {}
	void    OnActionFinished(EntityID entity);
	void    OnApChanged(EntityID entity, int deltaAp, int curAp, int maxAp) {}
	void    OnLeaderChanged(EntityID newLeaderEntity);
	// Target
	wstring BuildTargetRingKey(EntityID target) const;
	void    EnsureTargetRing(EntityID target);
	void    EnableTargetRingAt(EntityID target, BattleTeam team, int slotIdx, const TimelineUnitState& unit);
	void    DisableTargetRing(EntityID target);
	void    UpdateTargetHighlight();
	// WaitBar
	void    EnsureWaitBar();
	void    UpdateWaitBar();
	void    HideWaitBar();

	void UpdateLeaderHighlightFX(float dt);

private:
	TimelinePresenterConfig config{};
	LaneLayout              alliesLayout{};
	LaneLayout              enemiesLayout{};
	bool                    configured = false;

	EntityID                leaderEntity = 0;
	vector<_uint>           listenersId;
	bool                    subscriptionsWired = false;
	// Target
	EntityID                highlightedTarget = invalidEntity;

	const wstring targetRectKey   = L"target_rect";
	const wstring waitBarFrontKey = L"waittime_barfront";
	const wstring waitBarBackKey1 = L"waittime_barback1";
	const wstring waitBarBackKey2 = L"waittime_barback2";
	const wstring waitBarFullKey  = L"waittime_barfull";

	float barAnimDur = 0.08f;
	bool  showFullWhenExecuting = true;

	const wstring leaderHighlightKey = L"leader_icon_highlight";
	float         leaderHighlightT = 0.f;

private:
	SystemRegistry&        registry;
	BattleEventBus*        eventBus{};
	UIRegistry*            uiRegistry{};
	UIAnimSystem*          uiAnimSys{};
	BattleTimelineSystem*  timelineSys{};
	CharacterDataSystem*   dataSys{};
	BattleTargetSystem*    targetSys{};
	BattleAttributeSystem* attrSys{};
};

NS_END