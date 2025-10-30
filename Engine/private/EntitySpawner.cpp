#include "Enginepch.h"

EntitySpawner& EntitySpawner::NewEntity()
{
	handles = {};
	handles.entity = entities.Create();
	started = true;
	return *this;
}

EntitySpawner& EntitySpawner::WithTf(const TransformDesc& desc)
{
	auto& tfSys = registry.Get<TransformSystem>();
	handles.tf = tfSys.Create(handles.entity, desc);
	return *this;
}

EntitySpawner& EntitySpawner::WithPos(float x, float y, float z)
{
	if (handles.tf.IsValid())
		registry.Get<TransformSystem>().SetPos(handles.tf, x, y, z);
	return *this;
}

EntitySpawner& EntitySpawner::WithScale(float x, float y, float z)
{
	if (handles.tf.IsValid())
		registry.Get<TransformSystem>().SetScale(handles.tf, x, y, z);
	return *this;
}

EntitySpawner& EntitySpawner::WithEuler(float pitch, float yaw, float roll)
{
	if (handles.tf.IsValid())
		registry.Get<TransformSystem>().SetEuler(handles.tf, pitch, yaw, roll);
	return *this;
}

EntitySpawner& EntitySpawner::WithCam(float fovY, float aspect, float nearZ, float farZ, bool makeMainCam, RAYORIGIN rayPolicy)
{
	auto& camSys = registry.Get<CameraSystem>();
	handles.cam = camSys.Create(handles.entity, handles.tf, fovY, aspect, nearZ, farZ);
	camSys.SetRayPolicy(handles.cam, rayPolicy);
	if (makeMainCam) camSys.SetMainCam(handles.cam, true);
	return *this;
}

EntitySpawner& EntitySpawner::WithThirdCam(Handle targetTf, _fvec offset)
{
	if (handles.cam.IsValid())
		registry.Get<CameraSystem>().SetTarget(handles.cam, targetTf, offset);
	return *this;
}

EntitySpawner& EntitySpawner::WithThirdCam(Handle targetTf, _fvec offset, OffsetSpace offsetSpace, FollowPolicy followPolicy, float softDamping)
{
	if (!handles.cam.IsValid()) return *this;

	auto& camSys = registry.Get<CameraSystem>();
	camSys.SetTarget(handles.cam, targetTf, offset);
	camSys.SetFollowOffsetSpace(handles.cam, offsetSpace);
	camSys.SetFollowPolicy(handles.cam, followPolicy, softDamping);
	return *this;
}

EntitySpawner& EntitySpawner::WithOrbitCam(Handle targetTf, float initYaw, float initPitch, float initDist)
{
	if (!handles.cam.IsValid()) return *this;

	auto& orbitSys = registry.Get<OrbitCamSystem>();
	handles.orbitCam = orbitSys.Create(handles.entity, handles.cam, targetTf);

	if (auto data = orbitSys.Get(handles.orbitCam))
	{
		data->orbitYaw = initYaw;
		data->orbitPitch = clamp(initPitch, data->minPitch, data->maxPitch);
		data->orbitDist = initDist;
	}
	return *this;
}

EntitySpawner& EntitySpawner::WithFreeCam(float moveSpeed, float sensitivity)
{
	auto& freeCamSys = registry.Get<FreeCamSystem>();
	handles.freeCam = freeCamSys.Create(handles.entity, handles.tf, moveSpeed, sensitivity);
	return *this;
}

EntitySpawner& EntitySpawner::WithDirectionalLight(const LightProxy& desc)
{
	auto proxy = desc; proxy.type = ENUM(LIGHT::DIRECTIONAL);
	auto& lightSys = registry.Get<LightSystem>();
	handles.light = lightSys.Create(handles.entity, handles.tf, proxy);
	return *this;
}

EntitySpawner& EntitySpawner::WithPointLight(const LightProxy& desc)
{
	auto proxy = desc; proxy.type = ENUM(LIGHT::POINT);
	auto& lightSys = registry.Get<LightSystem>();
	lightSys.Create(handles.entity, handles.tf, proxy);
	return *this;
}

EntitySpawner& EntitySpawner::WithSpotLight(const LightProxy& desc)
{
	auto proxy = desc; proxy.type = ENUM(LIGHT::SPOT);
	auto& lightSys = registry.Get<LightSystem>();
	lightSys.Create(handles.entity, handles.tf, proxy);
	return *this;
}

EntitySpawner& EntitySpawner::WithLayer(_uint mask)
{
	auto& layerSys = registry.Get<LayerSystem>();
	handles.layer = layerSys.Create(handles.entity, handles.tf, mask);
	return *this;
}

EntitySpawner& EntitySpawner::WithModel(const wstring& modelKey)
{
	auto& modelSys = registry.Get<ModelSystem>();
	auto& tfSys = registry.Get<TransformSystem>();

	TransformData* tf = tfSys.Get(handles.tf);
	handles.model = modelSys.Create(handles.entity, handles.tf, modelKey);

	if (auto model = modelSys.Get(handles.model))
	{
		if (model->animator.IsValid())
			handles.animator = model->animator;
	}
	return *this;
}

EntitySpawner& EntitySpawner::WithAnimator(Skeleton* skeleton, const ClipTable* clips)
{
	auto& animSys = registry.Get<AnimatorSystem>();
	handles.animator = animSys.Create(handles.entity, skeleton, clips, handles.tf);
	return *this;
}

EntitySpawner& EntitySpawner::WithFace(Handle anim, wstring openClip, wstring closeClip, float openDur, float openJitter, float holdClose, float fadeClose, float fadeOpen)
{
	auto& faceSys = registry.Get<FaceSystem>();
	handles.face = faceSys.Create(handles.entity, anim,move(openClip), move(closeClip),openDur, openJitter, holdClose, fadeClose, fadeOpen);
	return *this;
}

EntitySpawner& EntitySpawner::WithMouth(Handle anim, const wstring& clip, _uint layer, float weight, float speed)
{
	auto& mouthSys = registry.Get<MouthSystem>();
	handles.mouth = mouthSys.Create(handles.entity, anim, clip, layer, weight, speed);
	return *this;
}

EntitySpawner& EntitySpawner::WithFace(wstring openClip, wstring closeClip, float openDur, float openJitter, float holdClose, float fadeClose, float fadeOpen)
{
	Handle anim = handles.animator;
	assert(anim.IsValid() && "WithFace: animator handle not available. Call WithModel or WithAnimator() first.");
	return WithFace(anim, move(openClip),move(closeClip), openDur, openJitter, holdClose, fadeClose, fadeOpen);
}

EntitySpawner& EntitySpawner::WithMouth(const wstring& clip, _uint layer, float weight, float speed)
{
	Handle anim = handles.animator;
	assert(anim.IsValid() && "WithMouth: animator handle not available. Call WithModel or WithAnimator() first.");
	return WithMouth(anim, clip, layer, weight, speed);
}

EntitySpawner& EntitySpawner::WithSocket(EntityID parentID, const string& boneName, const _float3& offsetPos, const _float3& offsetRot)
{
	auto& tfSys     = registry.Get<TransformSystem>();
	auto& socketSys = registry.Get<SocketSystem>();
	auto& animSys   = registry.Get<AnimatorSystem>();
	auto& modelSys  = registry.Get<ModelSystem>();

	Handle parentAnim{};
	animSys.ForEachOwned(parentID, [&](Handle handle, AnimData&)
		{
			if (!parentAnim.IsValid())
				parentAnim = handle;
		});

	Handle parentTf{};
	tfSys.ForEachOwned(parentID, [&](Handle handle, TransformData&)
		{
			if (!parentTf.IsValid())
				parentTf = handle;
		});

	if (!parentAnim.IsValid())
	{
		modelSys.ForEachOwned(parentID, [&](Handle handle, ModelData& model) 
			{
				if (!parentAnim.IsValid() && model.animator.IsValid())
					parentAnim = model.animator;
			});
	}
	assert(parentAnim.IsValid() && "WithSocket: parent entity has no Animator");
	if (!parentAnim.IsValid()) return *this;

	handles.socket = socketSys.Create(handles.entity, handles.tf, parentAnim, parentTf, boneName, offsetPos, offsetRot);
	return *this;
}

EntitySpawner& EntitySpawner::WithSocket(const string& parentTag, const string& boneName, const _float3& offsetPos, const _float3& offsetRot)
{
	auto& tags = registry.Get<TagSystem>();
	EntityID parentID = tags.Get(parentTag);
	assert(parentID != invalidEntity && "WithSocket: parent tag not found.");
	return WithSocket(parentID, boneName, offsetPos, offsetRot);
}

EntitySpawner& EntitySpawner::WithTag(const string& tag)
{
	registry.Get<TagSystem>().Register(handles.entity, tag);
	return *this;
}

EntitySpawner& EntitySpawner::WithGrid(const GridParams& params)
{
	auto& gridSys = registry.Get<GridSystem>();
	Handle grid   = gridSys.Create(handles.entity);
	gridSys.SetParams(grid, params);
	return *this;
}

EntitySpawner& EntitySpawner::WithGrid(float cellSize, int countX, int countZ, _float3 origin, int majorEvery, bool showMinor, bool showMajor, bool showHover)
{
	GridParams param{};
	param.cellSize   = max(1e-6f, cellSize);
	param.origin     = origin;
	param.majorEvery = max(1, majorEvery);
	param.showMinor  = showMinor;
	param.showMajor  = showMajor;
	param.showHover  = showHover;
	param.cellCountX = max(1, countX);
	param.cellCountZ = max(1, countZ);
	return WithGrid(param);
}

EntitySpawner& EntitySpawner::WithSelectable(_uint layerMask, bool enabled)
{
	auto& selectSys = registry.Get<SelectionSystem>();
	selectSys.Create(handles.entity, true, layerMask);
	return *this;
}

EntitySpawner& EntitySpawner::WithPickable(_uint layerMask, bool enabled)
{
	auto& pickSys = registry.Get<PickingSystem>();
	handles.picking = pickSys.Create(handles.entity, handles.tf, layerMask, enabled);
	return *this;
}

EntitySpawner& EntitySpawner::WithColliderAABB(const BoundingBox& localBox, _uint layerMask, bool enabled)
{
	auto& collisionSys = registry.Get<CollisionSystem>();
	Handle handle = collisionSys.CreateAABB(handles.entity, handles.tf, localBox, layerMask);
	if (!enabled)
		collisionSys.SetEnabled(handle, false);

	handles.collision = handle;
	return *this;
}

EntitySpawner& EntitySpawner::WithColliderSphere(const _float3& centerLocal, float radiusLocal, _uint layerMask, bool enabled)
{
	auto& collisionSys = registry.Get<CollisionSystem>();
	Handle handle = collisionSys.CreateSphere(handles.entity, handles.tf, centerLocal, radiusLocal, layerMask);
	if (!enabled)
		collisionSys.SetEnabled(handle, false);

	handles.collision = handle;
	return *this;
}

EntitySpawner& EntitySpawner::WithColliderOBB(const BoundingOrientedBox& localOBB, _uint layerMask, bool enabled)
{
	auto& collisionSys = registry.Get<CollisionSystem>();
	Handle handle = collisionSys.CreateOBB(handles.entity, handles.tf, localOBB, layerMask);
	if (!enabled)
		collisionSys.SetEnabled(handle, false);

	handles.collision = handle;
	return *this;
}

EntitySpawner& EntitySpawner::WithColliderFromModel(ColliderType type, _uint layerMask, bool enabled)
{
	BoundingBox modelAABB{};
	bool have = false;

	if (handles.model.IsValid())
	{
		auto& modelSys = registry.Get<ModelSystem>();
		if (const auto* comp = modelSys.Get(handles.model))
		{
			if (comp->model)
			{
				modelAABB = comp->model->GetBoundingBox();
				const auto extent = modelAABB.Extents;
				have = (extent.x > 0 || extent.y > 0 || extent.z > 0);
			}
		}
	}

	if (!have)
		modelAABB = BoundingBox({}, { 0.5f, 0.5f, 0.5f});

	auto& collisionSys = registry.Get<CollisionSystem>();
	Handle created{};

	switch (type)
	{
	case ColliderType::AABB:
		created = collisionSys.CreateAABB(handles.entity, handles.tf, modelAABB, layerMask);
		break;

	case ColliderType::Sphere:
	{
		const _float3 center = modelAABB.Center;
		const _float3 extent = modelAABB.Extents;
		const float radius = sqrtf(extent.x * extent.x + extent.y * extent.y + extent.z * extent.z);
		created = collisionSys.CreateSphere(handles.entity, handles.tf, center, radius, layerMask);
		break;

	}
	
	case ColliderType::OBB:
	{
		BoundingOrientedBox obb{};
		obb.Center = modelAABB.Center;
		obb.Extents = modelAABB.Extents;
		obb.Orientation = _float4{ 0, 0, 0, 1 }; // identity
		created = collisionSys.CreateOBB(handles.entity, handles.tf, obb, layerMask);
		break;
	}
	}

	if (!enabled)
		collisionSys.SetEnabled(created, false);
	handles.collision = created;
	return *this;
}

EntitySpawner& EntitySpawner::WithColliderPerPartAABB(_uint layerMask, bool enabled)
{
	auto& collisionSys = registry.Get<CollisionSystem>();

	if (handles.model.IsValid())
	{
		auto& modelSys = registry.Get<ModelSystem>();
		if (const auto* comp = modelSys.Get(handles.model))
		{
			if (comp->model)
			{
				for (const auto& part : comp->model->GetParts())
				{
					if (!part.mesh || !part.mesh->HasLocalBounds()) continue;
					const auto& box = part.mesh->GetLocalAABB();
					Handle handle = collisionSys.CreateAABB(handles.entity, handles.tf, box, layerMask);
					if (!enabled)
						collisionSys.SetEnabled(handle, false);
				}
			}
		}
	}
	return *this;
}

EntitySpawner& EntitySpawner::WithMeshCollider(_uint layerMask, bool enabled)
{
	if (!handles.model.IsValid()) return *this;

	auto& modelSys = registry.Get<ModelSystem>();
	Handle hModel{};
	const ModelData* model = modelSys.GetByOwner(handles.entity, &hModel);
	if (!model || !model->model) return *this;

	auto& mcSys = registry.Get<MeshColliderSystem>();
	mcSys.Create(handles.entity, model->transform, *model->model, layerMask, enabled);
	return *this;
}

EntitySpawner& EntitySpawner::WithPlayerMovement(const MoveProfile& preset)
{
	auto& profileSys   = registry.Get<MoveProfileSystem>();
	auto& stateSys     = registry.Get<MoveStateSystem>();
	auto& intentSys    = registry.Get<MoveIntentSystem>();
	auto& faceSys  = registry.Get<FacingSystem>();
	auto& fieldAnimSys = registry.Get<FieldAnimSystem>();
	auto& fieldCtrlSys = registry.Get<FieldControllerSystem>();
	auto& jumpSys      = registry.Get<JumpSystem>();
		
	profileSys.Create(handles.entity, preset);
	stateSys.Create(handles.entity, handles.tf);
	intentSys.Create(handles.entity);
	fieldAnimSys.Create(handles.entity, handles.animator);
	jumpSys.Create(handles.entity);

	Handle faceHandle = faceSys.Create(handles.entity);

	FacingParams facing{};
	facing.forwardOffsetRad = 0.f;

	handles.faceAnim = faceHandle;

	return *this;
}

EntitySpawner& EntitySpawner::WithSkybox(const wstring& modelKey, SkyTextureType type, bool attachToCam, float uniformScale, float baseYawRad, float rotSpeed, bool setActive)
{
	vector<SkySubmesh> submeshes = BuildSkysubmeshes(modelKey);
	if (submeshes.empty()) return *this;

	return WithSkybox(submeshes, type, attachToCam, uniformScale, baseYawRad, rotSpeed, setActive);
}

EntitySpawner& EntitySpawner::WithSkybox(const vector<SkySubmesh>& submeshList, SkyTextureType type, bool attachToCam, float uniformScale, float baseyawRad, float rotSpeed, bool setActive)
{
	if (submeshList.empty()) return *this;
	auto& skySys = registry.Get<SkyboxSystem>();

	Handle skyHandle = skySys.Create(handles.entity, handles.tf, submeshList, type, attachToCam, uniformScale, baseyawRad, rotSpeed);
	handles.skybox = skyHandle;

	if (setActive)
		skySys.SetActive(skyHandle, true);
	return *this;
}

vector<SkySubmesh> EntitySpawner::BuildSkysubmeshes(const wstring& modelKey)
{
	vector<SkySubmesh> out{};
	auto model = assets.GetModel(modelKey);
	if (!model) return out;

	_uint submeshIdx = 0;

	for (const auto& part : model->GetParts())
	{
		if (!part.mesh || !part.material) continue;

		shared_ptr<Material> material = part.material->Clone();
		MaterialMeta meta = material->GetMeta();
		meta.shaderKey = L"Skybox";
		
		material->SetMeta(meta);
		material->Resolve(assets.GetShaderCache(), assets.GetTextureCache());

		SkySubmesh subMesh{};
		subMesh.mesh     = part.mesh;
		subMesh.material = material;
		
		const SkyRule rule    = Utility::GetSkyRuleByIdx(submeshIdx);
		subMesh.queue         = rule.queue;
		subMesh.cull          = rule.cull;
		subMesh.transparent   = rule.transparent;
		subMesh.premultiplied = rule.premultiplied;

		if (subMesh.transparent)
			subMesh.opacity = 0.3f;

		out.emplace_back(move(subMesh));
		++submeshIdx;
	}
	return out;
}

EntityHandles EntitySpawner::Build()
{
	assert(started && "Call NewEntity() before adding components");
	EntityHandles out = handles;
	handles = {};
	started = false;
	return out;
}