#include "Enginepch.h"
#include "TimelineBarPresenter.h"

void TimelineBarPresenter::SetTrack(const _float2 startPos, const _float2& endPos)
{
	track.startPos = startPos;
	track.endPos   = endPos;
}

void TimelineBarPresenter::RebuildFromSession()
{
	icons.clear();

	//vector<EntityID> allyEntities = timelineSys.
}

void TimelineBarPresenter::Tick(float dt)
{

}

void TimelineBarPresenter::HideAll()
{
}

void TimelineBarPresenter::OnFullGauge(EntityID entity)
{
}

void TimelineBarPresenter::OnActionCommitted(EntityID entity)
{
}

void TimelineBarPresenter::OnDowned(EntityID entity)
{
}

void TimelineBarPresenter::OnLeaderChanged(EntityID newLeaderEntity, EntityID prevLeaderEntity)
{
}

const TimelineIconRuntime* TimelineBarPresenter::FindIcon(EntityID entity) const
{
	return nullptr;
}

void TimelineBarPresenter::ResolveOverlaps()
{
}
