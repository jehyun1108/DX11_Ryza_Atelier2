#pragma once

#include "NavMeshData.h"

NS_BEGIN(Engine)

class ENGINE_DLL PointMarkerSystem : public EntitySystem<NavPointMarker>
{
public:
	explicit PointMarkerSystem(SystemRegistry& registry) : EntitySystem(registry) {}

	Handle Create(EntityID owner, const _float3& localPos, _uint stableVertexIdx, EntityID parentID);
	
	void ClearAll(EntityID parentID);
	
	size_t Count(EntityID parentID) const;

	template<typename Func>
	void ForEach(EntityID parentID, Func&& func);

	void CollectRecent(EntityID parentID, size_t maxCount, vector<pair<Handle, const NavPointMarker*>>& outList) const;

private:
	_uint navCounter{};
};

template<typename Func>
inline void PointMarkerSystem::ForEach(EntityID parentID, Func&& func)
{
	ForEachAliveEx([&](Handle handle, EntityID owner, NavPointMarker& marker)
		{
			if (marker.parentID == parentID)
				func(owner, handle, marker);
		});
}

NS_END