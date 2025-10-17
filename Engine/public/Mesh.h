#pragma once

#include "MeshUtil.h"

NS_BEGIN(Engine)

class ENGINE_DLL Mesh 
{
public:
	HRESULT InitFromBuffers(ID3D11Device* device,const MeshMeta& meta, 
		const void* vtxData, _uint inVtxCount, _uint inVtxStride, 
		const void* idxData, _uint inIdxCount);

	HRESULT InitPrimitive(ID3D11Device* device, const MeshMeta& meta);

	void Bind(ID3D11DeviceContext* context) const;
	void Draw(ID3D11DeviceContext* context) const;

	// Utility
	_uint                  GetVtxCount()     const { return vtxCount; }
	_uint                  GetIdxCount()     const { return idxCount; }
	_uint                  GetVertexStride() const { return vtxStride; }
	DXGI_FORMAT            GetIdxFormat()    const { return idxFmt; }
	D3D_PRIMITIVE_TOPOLOGY GetTopology()     const { return topology; }
	VertexLayoutID         GetLayoutID()     const { return layoutID; }

	MESH                   GetMeshKind()     const { return meshKind; }
	MESHTYPE               GetUsage()        const { return usage; }
	PRIMITIVE              GetPrimitive()    const { return primitive; }
	bool                   IsRenderable()    const { return usage != MESHTYPE::Driver; }

	// Bounding
	bool                   HasLocalBounds()  const { return hasLocalBounds; }
	const BoundingBox&     GetLocalAABB()    const { return localAABB; }
	const BoundingSphere&  GetLocalSphere()  const { return localSphere; }

private:
	static _uint ComputeIdxStride(DXGI_FORMAT fmt);
	HRESULT      CreateVB(ID3D11Device* device, const void* data, _uint stride, _uint count);
	HRESULT      CreateIB(ID3D11Device* device, const void* data, _uint count, DXGI_FORMAT fmt);
	HRESULT      BuildPrimitive(const MeshMeta& meta, vector<uint8_t>& outVtx, vector<uint8_t>& outIdx,
		_uint& outVtxCount, _uint& outIdxCount, _uint& outVtxStride, DXGI_FORMAT& outIdxFmt) const;
	size_t       GetPosOffset(VertexLayoutID layout);

private:
	// GPU
	ComPtr<ID3D11Buffer> vb{};
	ComPtr<ID3D11Buffer> ib{};

	// Format
	_uint                  vtxCount  = 0;
	_uint                  idxCount  = 0;
	_uint                  vtxStride = 0;
	DXGI_FORMAT            idxFmt    = DXGI_FORMAT_R16_UINT;
	D3D_PRIMITIVE_TOPOLOGY topology  = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	VertexLayoutID         layoutID  = VertexLayoutID::Unknown;

	// Type
	MESH                   meshKind  = MESH::Static;
	MESHTYPE               usage     = MESHTYPE::Static;
	PRIMITIVE              primitive = PRIMITIVE::None;

	// Bounding
	bool                   hasLocalBounds = false;
	BoundingBox            localAABB{};
	BoundingSphere         localSphere{};
};

NS_END