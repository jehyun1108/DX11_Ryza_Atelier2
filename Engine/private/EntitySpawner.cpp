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
	auto& socketSys = registry.Get<SocketSystem>();
	auto& animSys   = registry.Get<AnimatorSystem>();
	auto& modelSys  = registry.Get<ModelSystem>();

	Handle parentAnim{};
	animSys.ForEachOwned(parentID, [&](Handle handle, AnimData&)
		{
			if (!parentAnim.IsValid())
				parentAnim = handle;
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

	handles.socket = socketSys.Create(handles.entity, handles.tf, parentAnim, boneName, offsetPos, offsetRot);
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

EntitySpawner& EntitySpawner::WithPicking(const BoundingBox& localBox, _uint layerMask, bool enabled)
{
	auto& pickSys = registry.Get<PickingSystem>();

	handles.picking = pickSys.Create(handles.entity, handles.tf, localBox, layerMask, enabled);
	return *this;
}

EntitySpawner& EntitySpawner::WithPickingFromModel(_uint layerMask, bool enabled)
{
	BoundingBox local{};
	bool haveAABB = false;

	if (handles.model.IsValid())
	{
		auto modelSys = registry.Get<ModelSystem>();
		if (auto* comp = modelSys.Get(handles.model))
		{
			if (comp->model)
			{
				local = comp->model->GetBoundingBox();

				const _float3 extent = local.Extents;

				if (extent.x > 0.f || extent.y > 0.f || extent.z > 0.f)
					haveAABB = true;

				if (haveAABB)
				{
					const float eps = 1e-4f;
					local.Extents.x = (local.Extents.x < eps) ? eps : local.Extents.x;
					local.Extents.y = (local.Extents.y < eps) ? eps : local.Extents.y;
					local.Extents.z = (local.Extents.z < eps) ? eps : local.Extents.z;
				}
			}
		}
	}
	if (!haveAABB)
		local = BoundingBox(_float3{}, _float3{ 0.5f, 0.5f, 0.5f });

	return WithPicking(local, layerMask, enabled);
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

EntityHandles EntitySpawner::Build()
{
	assert(started && "Call NewEntity() before adding components");
	EntityHandles out = handles;
	handles = {};
	started = false;
	return out;
}