#include "Enginepch.h"
#include "RenderSystem.h"

static inline BoundingBox TransformAABB(const BoundingBox& local, const _float4x4& world)
{
	BoundingBox worldBox;
	local.Transform(worldBox, XMLoadFloat4x4(&world));
	return worldBox;
}

template<typename T>
static inline _uint GetStableId(const T* ptr, unordered_map<const T*, _uint>& map, _uint& nextId)
{
	auto it = map.find(ptr);
	if (it != map.end()) return it->second;
	const _uint id = nextId++;
	map.emplace(ptr, id);
	return id;
}

// ---------------------------------------------------------------------------------------------

bool RenderSystem::FrustumCulling(const BoundingBox& worldAABB, const CameraProxy& cam) const
{
	// 1. Proj 으로 view-space frustum 생성
	BoundingFrustum frustumViewSpace;
	BoundingFrustum::CreateFromMatrix(frustumViewSpace, XMLoadFloat4x4(&cam.proj));

	// 2. Frustum을 월드 공간으로 변환
	BoundingFrustum frustumWorldSpace;
	frustumViewSpace.Transform(frustumWorldSpace, XMLoadFloat4x4(&cam.invView));

	// 3. 월드 AABB와 월드 Frustum 교차 판정
	return frustumWorldSpace.Contains(worldAABB) != ContainmentType::DISJOINT;
}

float RenderSystem::CalcCamDist(const _float4x4& world, const CameraProxy& cam) const
{
	const _mat worldMat = XMLoadFloat4x4(&world);
	const _mat viewMat = XMLoadFloat4x4(&cam.view);
	const _mat worldView = XMMatrixMultiply(worldMat, viewMat);

	const _vec pWorld = XMVectorSet(0, 0, 0, 1);
	const _vec pView = XMVector3TransformCoord(pWorld, worldView);
	return fabsf(XMVectorGetZ(pView));
}

void RenderSystem::BuildScene(RenderScene& out)
{
	out.Clear();

	// 1. Camera SnapShot
	{
		auto& camSys = registry.Get<CameraSystem>();
		Handle mainCam = camSys.GetMainCamHandle();
		CameraProxy cam{};
		camSys.ExtractCameraProxy(mainCam, cam);
		out.cam = cam;
	}

	// 2. Light SnapShot
	{
		auto& lightSys = registry.Get<LightSystem>();
		vector<LightProxy> lights;
		lightSys.ExtractLightProxies(lights);
		out.lights = move(lights);
	}

	// 3. ModelParts
	vector<RenderProxy> proxies;
	proxies.reserve(1024);

	auto& modelSys = registry.Get<ModelSystem>();
	auto& tfSys    = registry.Get<TransformSystem>();
	auto& layerSys = registry.Get<LayerSystem>();
	auto& animSys  = registry.Get<AnimatorSystem>();

	modelSys.ForEachAliveEx([&](Handle handle, EntityID owner, const ModelData& model)
		{
			if (!model.enabled || !model.model) return;

			const _float4x4* pWorld = tfSys.GetWorld(model.transform);
			if (!pWorld) return;

			if (auto layer = layerSys.GetByOwner(owner))
				if ((layer->layerMask & 0xFFFFFFFFu) == 0) return;

			// 각 파트 -> RenderProxy
			for (const auto& part : model.model->GetParts())
			{
				if (!part.mesh || !part.material) continue;
				if (part.mesh->GetUsage() == MESHTYPE::Driver) continue;

				RenderProxy proxy{};
				proxy.owner    = owner;
				proxy.mesh     = part.mesh;
				proxy.material = part.material;
				proxy.world    = *pWorld;

				// Skinning BoneMatrices 바인딩 정보
				if (model.animator.IsValid() && part.mesh->GetMeshKind() == MESH::Skeletal)
				{
					if (const auto matrices = animSys.GetFinalMatrices(model.animator))
					{
						if (!matrices->empty())
							proxy.boneMatrices = BoneMatrices{ matrices->data(), (_uint)matrices->size() };
					}
					proxy.skeleton = model.model->GetSkeleton();
				}

				// BoundingBox
				BoundingBox worldAABB;
				if (part.mesh->HasLocalBounds())
					worldAABB = TransformAABB(part.mesh->GetLocalAABB(), proxy.world);
				else
					worldAABB = BoundingBox({});

				// Frustum Culling
				if (!FrustumCulling(worldAABB, out.cam)) continue;

				// Sorting
				proxy.materialId = GetStableId(proxy.material.get(), materialIdMap, materialId);
				proxy.meshId     = GetStableId(proxy.mesh.get(), meshIdMap, meshId);

				// 투영 정렬용 거리
				proxy.camDistance = CalcCamDist(proxy.world, out.cam);
				proxies.emplace_back(move(proxy));
			}
		});

	// 4. Select / Highlight
	{
		auto& collisionSys = registry.Get<CollisionSystem>();
		out.drawColliders = collisionSys.IsDebugEnabled();
		if (out.drawColliders)
			collisionSys.ExtractColliderProxies(out.colliders);
	}

	// 5. Queue 분배 + 정렬
	auto& opaqueQueue = out.queues.opaque;
	auto& transQueue = out.queues.transparent;

	opaqueQueue.clear();
	transQueue.clear();

	opaqueQueue.reserve(proxies.size());
	transQueue.reserve(proxies.size() / 4);

	for (auto& proxy : proxies)
	{
		const bool isTransparent = (proxy.material && proxy.material->IsTransparent());

		DrawItem item{};
		item.proxy = move(proxy);

		if (isTransparent)
		{
			item.key.value = SortKey::Transparent(item.proxy.camDistance, item.proxy.materialId, item.proxy.meshId);
			transQueue.emplace_back(move(item));
		}
		else
		{
			item.key.value = SortKey::Opaque(item.proxy.materialId, item.proxy.meshId);
			opaqueQueue.emplace_back(move(item));
		}
	}

	auto byKey = [](const DrawItem& a, const DrawItem& b) {return a.key.value < b.key.value; };

	sort(opaqueQueue.begin(), opaqueQueue.end(), byKey);
	sort(transQueue.begin(),  transQueue.end(),  byKey);
}