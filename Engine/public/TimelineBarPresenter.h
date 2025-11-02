#pragma once

#include "TimelinePresenterData.h"

NS_BEGIN(Engine)

class TimelineBarPresenter
{
public:
	TimelineBarPresenter(UIRegistry& uiRegistry, UIAnimSystem& uiAnimSys, CharacterDataSystem& dataSys, BattleTimelineSystem& timelineSys)
		: uiRegistry(uiRegistry), uiAnimSys(uiAnimSys), dataSys(dataSys), timelineSys(timelineSys) {}

	// 세팅 전투 UI 초기화시 1회 호출
	void SetTrack(const _float2 startPos, const _float2& endPos);
	// 세션 시작후 유닛 스캔 -> 아이콘 생성/보증/텍스처 
	void RebuildFromSession();
	void Tick(float dt);
	void HideAll();

	// Event Trigger
	void OnFullGauge(EntityID entity);
	void OnActionCommitted(EntityID entity);
	void OnDowned(EntityID entity);
	void OnLeaderChanged(EntityID newLeaderEntity, EntityID prevLeaderEntity);

private:
	const TimelineIconRuntime* FindIcon(EntityID entity) const;
	// Icon 겹침 해소
	void  ResolveOverlaps();

private:
	UIRegistry&           uiRegistry;
	UIAnimSystem&         uiAnimSys;
	CharacterDataSystem&  dataSys;
	BattleTimelineSystem& timelineSys;

	TimelineTrack               track{};
	vector<TimelineIconRuntime> icons;
};

NS_END