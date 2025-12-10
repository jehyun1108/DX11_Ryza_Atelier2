#pragma once

#include "CollisionData.h"

NS_BEGIN(Engine)

class ENGINE_DLL CollisionSystem : public EntitySystem<CollisionData>, public IGuiRenderable
{
public:
	explicit CollisionSystem(SystemRegistry& registry) : EntitySystem(registry) {}
	void     OnBoot() override;

	// Create
	Handle CreateAABB(EntityID owner, Handle tfHandle, const BoundingBox& localBox);
	Handle CreateSphere(EntityID owner, Handle tfHandle, const _float3& centerLocal, float radiusLocal);
	Handle CreateOBB(EntityID owner, Handle tfHandle, const BoundingOrientedBox& localOBB);
	Handle CreateMeshRay(EntityID owner, Handle tfHandle, const Mesh* mesh, Mask belongs = Bit(CollisionLayer::Ground), Mask collides = kNone);

	// Runtime Switching
	void SetAABB(Handle handle, const BoundingBox& localBox);
	void SetSphere(Handle handle, const _float3& centerLocal, float radiusLocal);
	void SetOBB(Handle handle, const BoundingOrientedBox& localOBB);

	// Util
	void SetEnabled(Handle handle, bool enable);
	void SetTransform(Handle handle, Handle tfHandle);
	void SetBelongsTo(Handle handle, _uint layer);
	void SetCollidesWith(Handle handle, _uint mask);

	// Tick
	void   Tick(float dt);
	void   RenderGui(EntityID id) override;
	void   ExtractColliderProxies(vector<ColliderProxy>& out) const;
	RayHit RayCast(const RayDesc& ray) const;

	bool   GetObbTipPoint(EntityID owner, _float3& outTip) const;

	bool   FindWeaponHit(EntityID attacker, BattleHitInfo& outHit) const;

private:
	TransformSystem* tfSys{};
	ModelSystem*     modelSys{};
};

NS_END