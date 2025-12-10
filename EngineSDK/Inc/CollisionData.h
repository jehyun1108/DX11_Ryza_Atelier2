#pragma once

NS_BEGIN(Engine)
class Mesh;

enum class CollisionLayer : _uint
{
	Ground      = 1u << 0,
	Character   = 1u << 1,
	Prop        = 1u << 2,
	Trigger     = 1u << 3,
	RaycastOnly = 1u << 4,
};

using Mask = _uint;

constexpr Mask Bit(CollisionLayer layer)               { return static_cast<Mask>(layer); }
constexpr bool Has(Mask mask, CollisionLayer layer)    { return (mask & Bit(layer)) != 0; }
inline void Set(Mask& mask, CollisionLayer layer)   { mask |=  Bit(layer); }
inline void Clear(Mask& mask, CollisionLayer layer) { mask &= ~Bit(layer); }

constexpr Mask kNone = 0;
constexpr Mask kDefined = Bit(CollisionLayer::Ground)    |
                          Bit(CollisionLayer::Character) | 
                          Bit(CollisionLayer::Prop)      | 
                          Bit(CollisionLayer::Trigger)   | 
                          Bit(CollisionLayer::RaycastOnly);

constexpr Mask kAll = kDefined;
constexpr Mask kWorld = Bit(CollisionLayer::Ground) | Bit(CollisionLayer::Prop);

struct RayDesc
{
	_float3 origin;
	_float3 dir;
	float   maxDist;
	_uint   queryMask;
};
struct RayHit
{
	bool     hit = false;
	float    t   = FLT_MAX;
	_float3  pos{};
	_float3  normal{};
	Handle   handle{};
	EntityID owner{};
};
// =====================================================================================
enum class ColliderType : uint8_t { Sphere, AABB, OBB, MeshRay };
struct ColliderMesh
{
	const Mesh* mesh{};
	Handle      tf{};
	BoundingBox localAABB{};
	BoundingBox worldAABB{};
	_float4x4   worldMat{};
};
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
	ColliderType   type    = ColliderType::AABB;
	ColliderMesh   mesh{};
	bool           enabled = true;

	Handle         tf{};
	ColliderAABB   aabb{};
	ColliderSphere sphere{};
	ColliderOBB    obb{};

	Mask belongsTo    = Bit(CollisionLayer::Prop);
	Mask collidesWith = Bit(CollisionLayer::Ground) | Bit(CollisionLayer::Prop);
};
// =====================================
struct BattleHitInfo
{
	EntityID attacker{};
	EntityID target{};
	_float3  centerWorld{};
};

NS_END