#pragma once

NS_BEGIN(Engine)

struct MeshColliderData
{
	Handle          tf{};
	vector<_float3> posLocal;
	vector<_uint>   indices;
	BoundingBox     localAABB{};
	_uint layerMask = 0xFFFFFFFFu;
	bool enabled    = true;
};

NS_END