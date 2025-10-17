#pragma once

NS_BEGIN(Engine)

enum class MESHTYPE  : uint8_t { Animated, Static, Driver };
enum class MESH      : uint8_t { Static, Skeletal, Primitive };
enum class PRIMITIVE : uint8_t { None, Quad, Cube, Sphere, Cylinder, Capsule };

struct MeshMeta
{
	wstring    key;            
	wstring    source;     

	// Type
	MESH       meshKind   = MESH::Static;
	MESHTYPE   usage      = MESHTYPE::Static;
	PRIMITIVE  primitive  = PRIMITIVE::None;

	// Format
	VertexLayoutID           layout   = VertexLayoutID::Unknown;
	DXGI_FORMAT              idxFmt   = DXGI_FORMAT_R16_UINT;
	D3D_PRIMITIVE_TOPOLOGY   topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// Skeletal Only
	_uint                    maxBones = 0;

	// Bounding
	BoundingBox              localAABB{};
	BoundingSphere           localSphere{};

	// Primitive
	float sizeX = 1.f;
	float sizeY = 1.f;
	float sizeZ = 1.f;

	_uint segmentU = 16;
	_uint segmentV = 16;
};

NS_END