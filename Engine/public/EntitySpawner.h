#pragma once

#include "EntityHandles.h"

NS_BEGIN(Engine)

class ENGINE_DLL EntitySpawner
{
public:
	EntitySpawner(SystemRegistry& registry, EntityMgr& entities) : registry(registry), entities(entities) {}

	EntitySpawner& NewEntity();

// ---------- Transform --------------
	EntitySpawner& WithTf(const TransformDesc& desc = {});
	EntitySpawner& WithPos(float x, float y, float z);
	EntitySpawner& WithScale(float x, float y, float z);
	EntitySpawner& WithEuler(float pitch, float yaw, float roll);

// ------------ Camera -----------------
	EntitySpawner& WithCam(float fovY, float aspect , float nearZ = 0.1f, float farZ = 1000.f, bool makeMainCam = true, RAYORIGIN rayPolicy = RAYORIGIN::CameraPos);
	// (3rd)
	EntitySpawner& WithThirdCam(Handle targetTf, _fvec offset);

// ------------ FreeCam -------------------
	EntitySpawner& WithFreeCam(float moveSpeed = 200.f, float sensitivity = 0.25f);

// ----------- Light ----------------------
	EntitySpawner& WithDirectionalLight(const LightProxy& desc = {});
	EntitySpawner& WithPointLight(const LightProxy& desc);
	EntitySpawner& WithSpotLight(const LightProxy& desc);

// ------------- Layer ------------------
	EntitySpawner& WithLayer(_uint mask = 0xFFFFFFFFu);

// -------------- Model ------------------
	EntitySpawner& WithModel(const wstring& modelKey);

// ------------- Animator -----------------
	EntitySpawner& WithAnimator(Skeleton* skeleton, const ClipTable* clips);

// ---------------- Face & Mouth  -----------------------
private:
	EntitySpawner& WithFace(Handle anim, wstring openClip, wstring closeClip, float openDur = 2.5f, float openJitter = 1.2f, float holdClose = 0.12f, float fadeClose = 0.08f, float fadeOpen = 0.08f);
	EntitySpawner& WithMouth(Handle anim, const wstring& clip, _uint layer = 2, float weight = 1.f, float speed = 1.f);

public:
	EntitySpawner& WithFace(wstring openClip, wstring closeClip, float openDur = 2.5f, float openJitter = 1.2f, float holdClose = 0.12f, float fadeClose = 0.08f, float fadeOpen = 0.08f);
	EntitySpawner& WithMouth(const wstring& clip, _uint layer = 2, float weight = 1.f, float speed = 1.f);

// ------------------- Socket -------------
	EntitySpawner& WithSocket(EntityID parentID, const string& boneName, const _float3& offsetPos = {}, const _float3& offsetRot = {});
	EntitySpawner& WithSocket(const string& parentTag, const string& boneName, const _float3& offsetPos = {}, const _float3& offsetRot = {});

// ------------------ Tag ----------------
	EntitySpawner& WithTag(const string& tag);

	template<size_t N>
	EntitySpawner& WithTag(const char(&literal)[N])
	{
		registry.Get<TagSystem>().Register(handles.entity, string(literal, N - 1));
		return *this;
	}
// ------------------ Grid----------------------
	EntitySpawner& WithGrid(const GridParams& params);
	EntitySpawner& WithGrid(float cellSize = 10.f, int countX = 500, int countZ = 500, _float3 origin = {}, int majorEvery = 5, bool showMinor = true, bool showMajor = true, bool showHover = true);

// --------------- Picking ------------------
	EntitySpawner& WithPicking(const BoundingBox& localBox = { _float3{ 0, 0, 0 }, _float3{0.5f,0.5f,0.5f} }, _uint layerMask = 0xFFFFFFFFu, bool enabled = true);

	// 모델은 모델의 LocalAABB 사용
	EntitySpawner& WithPickingFromModel(_uint layerMask = 0xFFFFFFFFu, bool enabled = true);

// --------------- Collision ---------------------------
	EntitySpawner& WithColliderAABB(const BoundingBox& localBox, _uint layerMask = 0xFFFFFFFFu, bool enabled = true);
	EntitySpawner& WithColliderSphere(const _float3& centerLocal, float radiusLocal, _uint layerMask = 0xFFFFFFFFu, bool enabled = true);
	EntitySpawner& WithColliderOBB(const BoundingOrientedBox& localOBB, _uint layerMask = 0xFFFFFFFFu, bool enabled = true);
	EntitySpawner& WithColliderFromModel(ColliderType type = ColliderType::AABB, _uint layerMask = 0xFFFFFFFFu, bool enabled = true);
	EntitySpawner& WithColliderPerPartAABB(_uint layerMask = 0xFFFFFFFFu, bool enabled = true);

// -------------- Build ------------------
	EntityHandles Build();

private:
	SystemRegistry& registry;
	EntityMgr&      entities;
	EntityHandles   handles;
	bool            started = false; // NewEntity 로 시작?
};

NS_END