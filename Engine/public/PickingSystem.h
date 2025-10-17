#pragma once

#include "PickingData.h"

NS_BEGIN(Engine)

class ENGINE_DLL PickingSystem : public EntitySystem<PickingData>, public IGuiRenderable
{
public:
	explicit PickingSystem(SystemRegistry& registry) : EntitySystem(registry) {}

	Handle Create(EntityID owner, Handle transform, const BoundingBox& localBox, _uint layerMask = 0xFFFFFFFFu, bool enabled = true);

	void SetLocalBox(Handle handle, const BoundingBox& localBox);
	void SetEnabled(Handle handle, bool enabled);
	void SetLayerMask(Handle handle, _uint mask);

	void RebuildWorldAABB(Handle handle);
	void RebuildAll(Handle handle, _uint mask);

	void Update(float dt);

	// Raycast
	bool RaycastSingle(Handle handle, const _float3& origin, const _float3& dir, float& outDist) const;
	bool RaycastAll(const _float3& origin, const _float3& dir, _uint layerMask, PickingHit& outNearest);

	// Screen -> Ray -> Picking
	bool PickFromScreen(const PickingRequest& request, PickingHit& out) const;

	void RenderGui(EntityID id) override;
};

NS_END