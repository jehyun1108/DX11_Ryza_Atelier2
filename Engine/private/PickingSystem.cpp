#include "Enginepch.h"

Handle PickingSystem::Create(EntityID owner, Handle transform, _uint layerMask, bool enabled)
{
	Handle handle = CreateComp(owner);
	auto& comp = *Get(handle);
    comp.transform  = transform;
    comp.layerMask  = layerMask;
    comp.enabled    = enabled;
    comp.cacheWorld = _float4x4{};
	return handle;
}

void PickingSystem::SetEnabled(Handle handle, bool enabled)
{
	if (auto comp = Get(handle))
		comp->enabled = enabled;
}

void PickingSystem::SetLayerMask(Handle handle, _uint mask)
{
	if (auto comp = Get(handle))
		comp->layerMask = mask;
}

bool PickingSystem::MakeWorldRay(const PickingRequest& request, const CameraSystem& camSys, _float3& outOrigin, _float3& outDir)
{
	if (!request.fromScreen)
	{
		outOrigin = request.rayOrigin;
		const _vec vDir = XMVector3Normalize(XMLoadFloat3(&request.rayDir));
		XMStoreFloat3(&outDir, vDir);
		return true;
	}

	if (!camSys.Validate(request.cam))
		return false;

	_vec vOrigin{}, vDir{};
	camSys.CreateRayFromScreen(request.cam, request.screenpos, request.viewport, vOrigin, vDir);
	XMStoreFloat3(&outOrigin, vOrigin);
	XMStoreFloat3(&outDir, XMVector3Normalize(vDir));
	return true;
}

bool PickingSystem::Pick(const PickingRequest& request, PickingHit& outHit) const
{
	const auto& cameraSystem = registry.Get<CameraSystem>();

	_float3 originWorld{}, dirWorld{};
	if (!MakeWorldRay(request, cameraSystem, originWorld, dirWorld))
	{
		outHit = {};
		return false;
	}

	return RayCastAll(originWorld, dirWorld, request.layerMask, outHit);
}

bool PickingSystem::RayCastAll(const _float3& originWorld, const _float3& dirWorld, _uint layerMask, PickingHit& outNearest) const
{
	outNearest = {};
	outNearest.hit = false;
	outNearest.distance = FLT_MAX;

	bool anyHit = false;

	ForEachAliveEx([&](Handle handle, EntityID owner, const PickingData& comp)
		{
			if (!comp.enabled) return;
			if ((comp.layerMask & layerMask) == 0) return;

			PickingHit hit{};
			if (RayCastMeshCollider(owner, originWorld, dirWorld, hit))
			{
				if (hit.distance < outNearest.distance)
				{
					anyHit = true;
					outNearest = hit;
				}
			}
		});

	return anyHit;
}

bool PickingSystem::RayCastMeshCollider(EntityID entity, const _float3& originWorld, const _float3& dirWorld, PickingHit& outHit) const
{
	const MeshColliderData* collider = TryGetMeshCollider(entity);
	if (!collider || !collider->enabled) return false;

	auto& transformSystem = registry.Get<TransformSystem>();
	const TransformData* transformData = transformSystem.Get(collider->tf);
	if (!transformData) return false;

	const _mat worldMat    = transformData->dirty ? Utility::MakeWorldMat(*transformData) : XMLoadFloat4x4(&transformData->world);
	const _mat invWorldMat = XMMatrixInverse(nullptr, worldMat);

	const _vec vOriginWorld = XMLoadFloat3(&originWorld);
	const _vec vDirWorld    = XMVector3Normalize(XMLoadFloat3(&dirWorld));

	const _vec vOriginLocal = XMVector3TransformCoord(vOriginWorld, invWorldMat);
	const _vec vDirLocal    = XMVector3Normalize(XMVector3TransformNormal(vDirWorld, invWorldMat));

	_float3 originLocal{}; XMStoreFloat3(&originLocal, vOriginLocal);
	_float3 dirLocal{};    XMStoreFloat3(&dirLocal, vDirLocal);

	// 1. LocalAABB BroadCast
	float boxT = 0.f;
	if (!collider->localAABB.Intersects(vOriginLocal, vDirLocal, boxT)) return false;
	if (boxT < 0.f) return false;

	// ★ 2) 삼각형 루프 (최근접 t 상한)
	float  nearestT = FLT_MAX;
	_uint  nearestTriIndex = UINT32_MAX;
	float  nearestU = 0.f, nearestV = 0.f;

	const auto& pos       = collider->posLocal;
	const auto& indices   = collider->indices;
	const size_t triCount = indices.size() / 3;

	for (size_t tri = 0; tri < triCount; ++tri)
	{
		const _float3 a = pos[indices[tri * 3 + 0]];
		const _float3 b = pos[indices[tri * 3 + 1]];
		const _float3 c = pos[indices[tri * 3 + 2]];

		float t = 0.f, u = 0.f, v = 0.f;
		if (RayTriangleMT(originLocal, dirLocal, a, b, c, t, u, v))
		{
			if (t < nearestT)
			{
				nearestT = t;
				nearestTriIndex = static_cast<_uint>(tri);
				nearestU = u;
				nearestV = v;
			}
		}
	}

	if (nearestTriIndex == UINT32_MAX) return false;

	const _vec hitLocal = XMVectorMultiplyAdd(vDirLocal, XMVectorReplicate(nearestT), vOriginLocal);
	const _vec hitWorld = XMVector3TransformCoord(hitLocal, worldMat);

	const _float3 a = pos[indices[nearestTriIndex * 3 + 0]];
	const _float3 b = pos[indices[nearestTriIndex * 3 + 1]];
	const _float3 c = pos[indices[nearestTriIndex * 3 + 2]];
	const _vec abLocal = XMVectorSet(b.x - a.x, b.y - a.y, b.z - a.z, 0.0f);
	const _vec acLocal = XMVectorSet(c.x - a.x, c.y - a.y, c.z - a.z, 0.0f);
	const _vec nLocal = XMVector3Normalize(XMVector3Cross(abLocal, acLocal));
	const _vec nWorld = XMVector3Normalize(XMVector3TransformNormal(nLocal, worldMat));

	outHit.hit = true;
	outHit.entity = entity;
	outHit.distance = XMVectorGetX(XMVector3Length(hitWorld - vOriginWorld));
	XMStoreFloat3(&outHit.point, hitWorld);
	XMStoreFloat3(&outHit.normal, nWorld);
	outHit.triangleIdx = nearestTriIndex;
	outHit.uv = _float2{ nearestU, nearestV };
	return true;
}

bool PickingSystem::RayTriangleMT(const _float3& localRayOrigin, const _float3& localRayDir, const _float3& vertexA, const _float3& vertexB, const _float3& vertexC, float& outRayDistance, float& outBarycentricU, float& outBarycentricV)
{
	constexpr float epsilon = 1e-7f;

	const _float3 edgeAtoB{ vertexB.x - vertexA.x, vertexB.y - vertexA.y, vertexB.z - vertexA.z };
	const _float3 edgeAtoC{ vertexC.x - vertexA.x, vertexC.y - vertexA.y, vertexC.z - vertexA.z };

	const _float3 dirCrossEdgeAtoC {
		localRayDir.y * edgeAtoC.z - localRayDir.z * edgeAtoC.y,
		localRayDir.z * edgeAtoC.x - localRayDir.x * edgeAtoC.z,
		localRayDir.x * edgeAtoC.y - localRayDir.y * edgeAtoC.x
	};

	const float determinant =
		edgeAtoB.x * dirCrossEdgeAtoC.x +
		edgeAtoB.y * dirCrossEdgeAtoC.y +
		edgeAtoB.z * dirCrossEdgeAtoC.z;

	if (fabsf(determinant) < epsilon) return false;

	const float invDeterminant = 1.0f / determinant;

	const _float3 originToAVector{
		localRayOrigin.x - vertexA.x,
		localRayOrigin.y - vertexA.y,
		localRayOrigin.z - vertexA.z
	};

	const float baryU = (originToAVector.x * dirCrossEdgeAtoC.x + originToAVector.y * dirCrossEdgeAtoC.y + originToAVector.z * dirCrossEdgeAtoC.z) * invDeterminant;

	if (baryU < -1e-6f || baryU > 1.0f + 1e-6f)
		return false;

	const _float3 tCrossEdgeAtoB {
		originToAVector.y * edgeAtoB.z - originToAVector.z * edgeAtoB.y,
		originToAVector.z * edgeAtoB.x - originToAVector.x * edgeAtoB.z,
		originToAVector.x * edgeAtoB.y - originToAVector.y * edgeAtoB.x
	};

	const float baryV = (localRayDir.x * tCrossEdgeAtoB.x + localRayDir.y * tCrossEdgeAtoB.y + localRayDir.z * tCrossEdgeAtoB.z) * invDeterminant;

	if (baryV < -1e-6f || (baryU + baryV) > 1.0f + 1e-6f) return false;

	const float rayDistance = (edgeAtoC.x * tCrossEdgeAtoB.x + edgeAtoC.y * tCrossEdgeAtoB.y + edgeAtoC.z * tCrossEdgeAtoB.z) * invDeterminant;

	if (rayDistance < 0.0f) return false;

	outRayDistance = rayDistance;
	outBarycentricU = baryU;
	outBarycentricV = baryV;
	return true;
}

const MeshColliderData* PickingSystem::TryGetMeshCollider(EntityID owner) const
{
	const auto& mcSys = registry.Get<MeshColliderSystem>();
	return mcSys.TryGetByOwner(owner, nullptr);
}

void PickingSystem::RenderGui(EntityID id)
{
#ifdef USE_IMGUI
// ------------------------------------------------------------------------------------------
	
#endif
}

