#include "Enginepch.h"
#include "UIMinimapSystem.h"

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
	BoundingFrustum frustumViewSpace;
	BoundingFrustum::CreateFromMatrix(frustumViewSpace, XMLoadFloat4x4(&cam.proj));
	BoundingFrustum frustumWorldSpace;
	frustumViewSpace.Transform(frustumWorldSpace, XMLoadFloat4x4(&cam.invView));
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

float RenderSystem::CalcCamDist(const _float3& pos, const CameraProxy& cam) const
{
	const _mat viewMat = XMLoadFloat4x4(&cam.view);
	const _vec pWorld  = XMLoadFloat3(&pos);
	const _vec pView   = XMVector3TransformCoord(pWorld, viewMat);
	return fabsf(XMVectorGetZ(pView));
}

void RenderSystem::OnBoot()
{
	camSys       = &registry.Get<CameraSystem>();
	lightSys     = &registry.Get<LightSystem>();
	skySys       = &registry.Get<SkyboxSystem>();
	uiSys        = &registry.Get<UISystem>();
	modelSys     = &registry.Get<ModelSystem>();
	tfSys        = &registry.Get<TransformSystem>();
	layerSys     = &registry.Get<LayerSystem>();
	animator     = &registry.Get<AnimatorSystem>();
	collisionSys = &registry.Get<CollisionSystem>();
	miniSys      = &registry.Get<UIMinimapSystem>();
	particleSys  = &registry.Get<ParticleSystem>();
	trailSys     = &registry.Get<TrailSystem>();
	effectSys    = &registry.Get<EffectSystem>();
}

void RenderSystem::BuildScene(RenderScene& out)
{
	out.Clear();

	camSys->ExtractCameraProxy(camSys->GetMainCamHandle(), out.cam);
	lightSys->ExtractLightProxies(out.lights);
	skySys->ExtractSkyboxProxies(out.skybox);
	uiSys->ExtractUIProxies(out.ui);
	particleSys->ExtractParticleSnapshot(out.particles, out.cam);
	trailSys->ExtractTrailSnapshot(out.trails, out.cam);
	
	out.minimapEnabled = (miniSys->GetMode() != MinimapMode::None);
	if (out.minimapEnabled)
		out.minimapCam = miniSys->GetCamera();

	// 3. ModelParts
	vector<RenderProxy> proxies;
	proxies.reserve(1024);
	modelSys->ForEachAliveEx([&](Handle handle, EntityID owner, const ModelData& model)
		{
			if (!model.enabled || !model.model) return;

			const _float4x4* pWorld = tfSys->GetWorld(model.transform);
			_uint layerMask = layerSys->GetByOwner(owner)->layerMask;
			if (layerMask == 0) return;
			// 각 파트 -> RenderProxy
			for (const auto& part : model.model->GetParts())
			{
				if (!part.mesh || !part.material || part.mesh->GetUsage() == MESHTYPE::Driver) continue;

				RenderProxy proxy{};
				proxy.owner     = owner;
				proxy.mesh      = part.mesh;
				proxy.material  = part.material;
				proxy.world     = *pWorld;
				proxy.isSkinned = (part.mesh->GetLayoutID() == VertexLayoutID::PNUTanSkin);
				proxy.layerMask = layerMask;

				if (proxy.isSkinned)
				{
					const vector<_float4x4>* finalMatrices{};
					if (model.animator.IsValid())
						finalMatrices = animator->GetFinalMatrices(model.animator);

					if (!finalMatrices || finalMatrices->empty())
						finalMatrices = &model.model->GetBindPoseMatrices();

					if (finalMatrices && !finalMatrices->empty())
						proxy.boneMatrices = BoneMatrices{ finalMatrices->data(), static_cast<_uint>(finalMatrices->size()) };

					proxy.skeleton = model.model->GetSkeleton();
				}
				BoundingBox worldAABB;
				if (part.mesh->HasLocalBounds())
					worldAABB = TransformAABB(part.mesh->GetLocalAABB(), proxy.world);
				else
					worldAABB = BoundingBox({});

				if (!FrustumCulling(worldAABB, out.cam)) continue;

				proxy.materialId  = GetStableId(proxy.material.get(), materialIdMap, materialId);
				proxy.meshId      = GetStableId(proxy.mesh.get(), meshIdMap, meshId);
				proxy.camDistance = CalcCamDist(proxy.world, out.cam);
				proxies.emplace_back(move(proxy));
			}
		});

	{	 //4. Select / Highlight
		out.drawColliders = false;
#ifdef USE_IMGUI
		out.drawColliders = true;
#endif
		if (out.drawColliders)
			collisionSys->ExtractColliderProxies(out.colliders);
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
		item.layerMask = proxy.layerMask;
		item.proxy     = move(proxy);

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
	auto byKey = [](const DrawItem& a, const DrawItem& b) { return a.key.value < b.key.value; };

	sort(opaqueQueue.begin(), opaqueQueue.end(), byKey);
	sort(transQueue.begin(),  transQueue.end(),  byKey);
}