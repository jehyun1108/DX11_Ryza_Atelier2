#pragma once

#include "CollisionData.h"

NS_BEGIN(Engine)
class Mesh;
class Material;
class Skeleton;

struct RenderProxy
{
	_uint                  owner = 0;
	shared_ptr<Mesh>       mesh{};
	shared_ptr<Material>   material{};
	_float4x4              world{};

	bool                   isSkinned = false;
	optional<BoneMatrices> boneMatrices{};
	shared_ptr<Skeleton>   skeleton{};
	
	float                  camDistance = 0.f;
	_uint                  materialId  = 0;
	_uint                  meshId      = 0;
	_uint                  layerMask   = 0;
};
struct ColliderProxy
{
	ColliderType type  = ColliderType::AABB;

	// SnapShot
	BoundingBox         aabb{};
	BoundingOrientedBox obb{};
	_float3             sphereCenter{};
	float               sphereRadius = 0.f;
};
// -------------------------------------
enum class SkyCull  : uint8_t {Back, Front, None};
enum class SkyQueue : uint8_t {Opaque, Alpha};
struct SkySubmesh
{
	shared_ptr<Mesh>     mesh;
	shared_ptr<Material> material;

	SkyQueue queue = SkyQueue::Opaque;
	SkyCull  cull  = SkyCull::Front;

	bool transparent   = false; 
	bool premultiplied = false;

	float opacity = 1.f;
};
struct SkyDrawLists
{
	vector<const SkySubmesh*> opaque;
	vector<const SkySubmesh*> alpha;
};
struct SkyboxProxy
{
	bool enabled = false;
	vector<SkySubmesh> submeshes;
	SkyTextureType textureType = SkyTextureType::Equirect2D;

	bool  attachToCamera = true;
	float uniformScale   = 1000.f;

	float baseYawRad = 0.f; // 시작 각도(초기 방향)
	float rotSpeed   = 0.02f;
	float phaseRad   = 0.f; // 누적 각도 

	bool  hasTfYaw   = false;
	float tfYawRad   = 0.f;
};
// ========================================


NS_END