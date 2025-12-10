#include "Enginepch.h"

Handle MeshColliderSystem::Create(EntityID owner, Handle tfHandle, const Model& model, _uint layerMask, bool enabled)
{
	Handle handle  = CreateComp(owner);
	auto& data     = *Get(handle);
	data.tf        = tfHandle;
	data.layerMask = layerMask;
	data.enabled   = enabled;
	
	ExtractCPUFromModel(model, data.posLocal, data.indices);

	if (!data.posLocal.empty())
	{
		BoundingBox::CreateFromPoints(data.localAABB, static_cast<size_t>(data.posLocal.size()), reinterpret_cast<const _float3*>(data.posLocal.data()), sizeof(_float3));

		const float eps = 1e-4f;
		data.localAABB.Extents.x = max(data.localAABB.Extents.x, eps);
		data.localAABB.Extents.y = max(data.localAABB.Extents.y, eps);
		data.localAABB.Extents.z = max(data.localAABB.Extents.z, eps);
	}

	return handle;
}

void MeshColliderSystem::ExtractCPUFromModel(const Model& model, vector<_float3>& outPos, vector<_uint>& outIndices)
{
	outPos.clear();
	outIndices.clear();

	_uint baseVertex = 0;

	for (const ModelParts& part : model.GetParts())
	{
		const shared_ptr<Mesh>& mesh = part.mesh;

		vector<_float3> partPos;
		vector<_uint> partIndices;
		if (!CopyPosAndIndicesFromMesh(*mesh, partPos, partIndices)) continue;

		outPos.insert(outPos.end(), partPos.begin(), partPos.end());
		for (_uint idx : partIndices)
			outIndices.push_back(baseVertex + idx);

		baseVertex += static_cast<_uint>(partPos.size());
	}
}

bool MeshColliderSystem::CopyPosAndIndicesFromMesh(const Mesh& mesh, vector<_float3>& outPos, vector<_uint>& outIndices)
{
	const uint8_t* cpuVB = mesh.GetCPUVertexBytes();
	const uint8_t* cpuIB = mesh.GetCPUIndexBytes();
	if (!cpuVB || !cpuIB) return false;

	const _uint  vtxCount    = mesh.GetVtxCount();
	const _uint  vtxStride   = mesh.GetVertexStride();
	const DXGI_FORMAT idxFmt = mesh.GetIdxFormat();
	const _uint  idxCount    = mesh.GetIdxCount();

	outPos.resize(vtxCount);
	for (_uint i = 0; i < vtxCount; ++i)
	{
		const _float3* pos = reinterpret_cast<const _float3*>(cpuVB + i * vtxStride);
		outPos[i] = *pos;
	}

	outIndices.resize(idxCount);
	if (idxFmt == DXGI_FORMAT_R16_UINT)
	{
		const uint16_t* src = reinterpret_cast<const uint16_t*>(cpuIB);
		for (_uint i = 0; i < idxCount; ++i)
			outIndices[i] = static_cast<_uint>(src[i]);
	}
	else
	{
		const uint32_t* src = reinterpret_cast<const uint32_t*>(cpuIB);
		for (_uint i = 0; i < idxCount; ++i)
			outIndices[i] = static_cast<_uint>(src[i]);
	}
	return true;
}