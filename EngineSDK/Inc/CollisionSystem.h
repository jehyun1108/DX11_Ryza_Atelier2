#pragma once

#include "CollisionData.h"

NS_BEGIN(Engine)

class ENGINE_DLL CollisionSystem : public EntitySystem<CollisionData>, public IGuiRenderable
{
public:
	explicit CollisionSystem(SystemRegistry& registry) : EntitySystem(registry) {}

	// Create
	Handle CreateAABB(EntityID owner, Handle tfHandle, const BoundingBox& localBox, _uint layer = ~0u);
	Handle CreateSphere(EntityID owner, Handle tfHandle, const _float3& centerLocal, float radiusLocal, _uint layer = ~0u);
	Handle CreateOBB(EntityID owner, Handle tfHandle, const BoundingOrientedBox& localOBB, _uint layer = ~0u);

	// Runtime Switching
	void SetAABB(Handle handle, const BoundingBox& localBox);
	void SetSphere(Handle handle, const _float3& centerLocal, float radiusLocal);
	void SetOBB(Handle handle, const BoundingOrientedBox& localOBB);

	// Debug ¿ë
	void SetDebugEnabled(bool on) { debugEnabled = on; }
	bool IsDebugEnabled()   const { return debugEnabled; }

	// Util
	void SetEnabled(Handle handle, bool enable);
	void SetLayer(Handle handle, _uint mask);
	void SetTransform(Handle handle, Handle tfHandle);

	// Tick
	void Update(float dt);
	void RenderGui(EntityID id) override;
	void ExtractColliderProxies(vector<ColliderProxy>& out) const;

private:
	bool debugEnabled = true;
};

NS_END