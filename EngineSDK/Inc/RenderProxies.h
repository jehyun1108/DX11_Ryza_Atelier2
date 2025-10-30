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
};

struct LightProxy
{
	int     type = ENUM(LIGHT::DIRECTIONAL);
	_float  range = 100.f;
	_float  spotAngle = XM_PI / 4.f;
	_float  padding{};
	_float4 ambient = { 0.2f, 0.2f, 0.2f, 1.f };
	_float4 diffuse = { 0.98f, 0.98f, 0.98f, 1.f };
	_float4 specular = { 0.3f, 0.3f, 0.3f, 1.f};
	_float4 lightPos;
	_float4 lightDir = { 0.f, -1.f, 0.f, 0.f };
};

struct CameraProxy
{
	_float4x4 view;
	_float4x4 proj;
	_float4x4 invView;
	_float4   camPos;
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

	// Camera 연동 (보통 배경)
	bool  attachToCamera = true;
	float uniformScale = 1000.f;

	float baseYawRad = 0.f; // 시작 각도(초기 방향)
	float rotSpeed = 0.02f;
	float phaseRad = 0.f; // 누적 각도 

	bool  hasTfYaw = false;
	float tfYawRad = 0.f;
};

NS_END