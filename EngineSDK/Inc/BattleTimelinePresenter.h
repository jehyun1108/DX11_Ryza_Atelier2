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

private:
	void  EnsureConfigured();
	void  ComputeLayoutsFromViewport();

	float ResolveXPos(const TimelineUnitState& unitState, bool isEnemy) const;
	int   ComputeZBias(bool isLeader, float progress01, int tieBreaker) const;
	int   ComposeZOrder(BattleTeam team, bool isLeader, float progress01, int tieBreaker) const;

	// 리더 연출
	void ApplyLeaderHighlight();
	void ClearLeaderHighlight();

	bool EnsureIconInstance(EntityID entity, BattleTeam team, int slotIdx);
	void SetIconEnabled(EntityID entity, bool enabled);

	wstring        BuildInstanceKey(EntityID entity) const;
	const wstring* ResolveIconKey(EntityID entity) const;

	void WireEventSubscriptions();
	void UnWireEventSubscriptions();

	void OnFullGauge(EntityID entity) {}
	void OnActionCommitted(EntityID entity) {}
	void OnActionFinished(EntityID entity);
	void OnApChanged(EntityID entity, int deltaAp, int curAp, int maxAp) {}
	void OnLeaderChanged(EntityID newLeaderEntity);

private:
	SystemRegistry&       registry;
	BattleEventBus*       eventBus{};
	UIRegistry*           uiRegistry{};
	UIAnimSystem*         uiAnimSys{};
	BattleTimelineSystem* timelineSys{};
	CharacterDataSystem*  dataSys{};

	TimelinePresenterConfig config{};
	LaneLayout              alliesLayout{};
	LaneLayout              enemiesLayout{};
	bool                    configured = false;

	EntityID                leaderEntity = 0;
	vector<_uint>           listenersId;
	bool                    subscriptionsWired = false;
};

NS_END