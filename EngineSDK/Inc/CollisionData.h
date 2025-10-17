#pragma once

NS_BEGIN(Engine)

enum class ColliderType : uint8_t {Sphere, AABB, OBB };

struct ColliderAABB
{
	BoundingBox local;
	BoundingBox world;
};

struct ColliderSphere
{
	_float3 centerLocal{};
	_float  radiusLocal{};
	_float3 centerWorld{};
	_float  radiusWorld{};
};

struct ColliderOBB
{
	BoundingOrientedBox local{};
	BoundingOrientedBox world{};
};

struct CollisionData
{
	ColliderType   type      = ColliderType::AABB;
	bool           enabled   = true;
	_uint          layerMask = 0xFFFFFFFFu;

	Handle         tf{};
	ColliderAABB   aabb{};
	ColliderSphere sphere{};
	ColliderOBB    obb{};
};

NS_END