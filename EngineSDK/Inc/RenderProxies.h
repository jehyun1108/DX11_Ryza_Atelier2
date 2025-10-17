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

	_float4 ambient = { 0.3f, 0.3f, 0.3f, 0.3f };
	_float4 diffuse = { 1.f, 1.f, 1.f, 1.f };
	_float4 specular = { 1.f, 1.f, 1.f, 1.f };
	_float4 lightPos;
	_float4 lightDir = { 0.f, -1.f, 0.f, 0.f };
};

struct CameraProxy
{
	_float4x4 view;
	_float4x4 proj;
	_float4x4 invView;
	_float4 camPos;
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

NS_END