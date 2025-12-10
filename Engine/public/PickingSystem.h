#pragma once

#include "PickingData.h"

NS_BEGIN(Engine)

class ENGINE_DLL PickingSystem : public EntitySystem<PickingData>, public IGuiRenderable
{
public:
	explicit PickingSystem(SystemRegistry& registry) : EntitySystem(registry) {}
	void     OnBoot() override;

	Handle Create(EntityID owner, Handle transform, _uint layerMask = 0xFFFFFFFFu, bool enabled = true);

	void SetEnabled(Handle handle, bool enabled);
	void SetLayerMask(Handle handle, _uint mask);
	bool Pick(const PickingRequest& request, PickingHit& outHit);
	bool RayCastAll(const _float3& originWorld, const _float3& dirWorld, _uint layerMask, PickingHit& outNearest);
	void RenderGui(EntityID id) override;

private:
	bool MakeWorldRay(const PickingRequest& request, _float3& outOrigin, _float3& outDir) const;
	bool RayCastMeshCollider(EntityID entity, const _float3& worldOrigin, const _float3& worldDir, PickingHit& outHit) const;
	static bool RayTriangleMT(const _float3& localRayOrigin, const _float3& localRayDir, const _float3& vertexA, const _float3& vertexB, const _float3& vertexC, float& outRayDistance, float& outBarycentricU, float& outBarycentricV);
	const struct MeshColliderData* TryGetMeshCollider(EntityID owner) const;

private:
	TransformSystem*    tfSys{};
	MeshColliderSystem* mcSys{};
	CameraSystem*       camSys{};
};

NS_END