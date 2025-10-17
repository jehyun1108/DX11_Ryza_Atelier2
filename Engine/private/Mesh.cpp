#include "Enginepch.h"

HRESULT Mesh::InitFromBuffers(ID3D11Device* device, const MeshMeta& meta,
	const void* vtxData, _uint inVtxCount, _uint inVtxStride, const void* idxData, _uint inIdxCount)
{
	assert(vtxData && idxData);
	assert(inVtxCount > 0 && inVtxStride > 0 && inIdxCount > 0);
	assert(meta.layout != VertexLayoutID::Unknown);

	meshKind  = meta.meshKind;
	usage     = meta.usage;
	primitive = meta.primitive;
	layoutID  = meta.layout;
	topology  = meta.topology;
	idxFmt    = meta.idxFmt;
	vtxCount  = inVtxCount;
	vtxStride = inVtxStride;
	idxCount  = inIdxCount;

	// VB / IB
	HR(CreateVB(device, vtxData, vtxStride, vtxCount));
	HR(CreateIB(device, idxData, idxCount, idxFmt));

	if (meta.localSphere.Radius > 0.f)
	{
		localAABB      = meta.localAABB;
		localSphere    = meta.localSphere;
		hasLocalBounds = true;
	}

	{
		const uint8_t* base    = reinterpret_cast<const uint8_t*>(vtxData);
		const size_t posOffset = GetPosOffset(layoutID);

		BoundingBox::CreateFromPoints(localAABB, vtxCount, reinterpret_cast<const _float3*>(base + posOffset), static_cast<size_t>(vtxStride));

		const _float3& extent = localAABB.Extents;
		const float radius = sqrtf(extent.x * extent.x + extent.y * extent.y + extent.z * extent.z);
		localSphere = BoundingSphere(localAABB.Center, radius);

		hasLocalBounds = (localSphere.Radius > 0.f);
	}

	return S_OK;
}

HRESULT Mesh::InitPrimitive(ID3D11Device* device, const MeshMeta& meta)
{
	assert(device && meta.meshKind == MESH::Primitive && meta.layout != VertexLayoutID::Unknown);

	meshKind  = MESH::Primitive;
	usage     = meta.usage;
	primitive = meta.primitive;
	layoutID  = meta.layout;
	topology  = meta.topology;

	if (layoutID != VertexLayoutID::PNUTan) 
		return E_INVALIDARG;

	vector<uint8_t> vtx, idx;
	_uint vCount = 0;
	_uint iCount = 0;
	_uint stride = 0;
	DXGI_FORMAT idxFormat = DXGI_FORMAT_R16_UINT;

	HR(BuildPrimitive(meta, vtx, idx, vCount, iCount, stride, idxFormat));

	vtxCount  = vCount;
	idxCount  = iCount;
	vtxStride = stride;
	idxFmt    = idxFormat;

	// Bounding
	if (localSphere.Radius <= 0.f && meta.localSphere.Radius > 0.f)
	{
		localAABB   = meta.localAABB;
		localSphere = meta.localSphere;
		hasLocalBounds = true;
	}
	else if (localSphere.Radius <= 0.f)
	{
		const float radiusX = meta.sizeX * 0.5f;
		const float radiusY = meta.sizeY * 0.5f;
		const float radiusZ = meta.sizeZ * 0.5f;
		localAABB = BoundingBox({}, { radiusX, radiusY, radiusZ });
		localSphere = BoundingSphere({}, sqrtf(radiusX * radiusX + radiusY * radiusY + radiusZ * radiusZ));
		hasLocalBounds = true;
	}

	HR(CreateVB(device, vtx.data(), vtxStride, vtxCount));
	HR(CreateIB(device, idx.data(), idxCount, idxFmt));


	return S_OK;
}

void Mesh::Bind(ID3D11DeviceContext* context) const
{
	assert(vb && ib && "Mesh::Bind - buffers not created");
	_uint offset = 0;
	_uint stride = vtxStride;
	context->IASetVertexBuffers(0, 1, vb.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(ib.Get(), idxFmt, 0);
	context->IASetPrimitiveTopology(topology);
}

void Mesh::Draw(ID3D11DeviceContext* context) const
{
	assert(idxCount > 0);
	context->DrawIndexed(idxCount, 0, 0);
}

_uint Mesh::ComputeIdxStride(DXGI_FORMAT fmt)
{
	switch (fmt)
	{
	case DXGI_FORMAT_R16_UINT: return 2;
	case DXGI_FORMAT_R32_UINT: return 4;
	default:                   return 0;
	}
	
}

HRESULT Mesh::CreateVB(ID3D11Device* device, const void* data, _uint stride, _uint count)
{
	D3D11_BUFFER_DESC desc{};
	desc.ByteWidth = stride * count;
	desc.Usage     = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA sub{};
	sub.pSysMem = data;

	return device->CreateBuffer(&desc, &sub, vb.GetAddressOf());
}

HRESULT Mesh::CreateIB(ID3D11Device* device, const void* data, _uint count, DXGI_FORMAT fmt)
{
	D3D11_BUFFER_DESC desc{};
	desc.ByteWidth = ComputeIdxStride(fmt) * count;
	desc.Usage     = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	
	D3D11_SUBRESOURCE_DATA sub{};
	sub.pSysMem = data;

	return device->CreateBuffer(&desc, &sub, ib.GetAddressOf());
}

size_t Mesh::GetPosOffset(VertexLayoutID layout)
{
	switch (layout)
	{
	case VertexLayoutID::Unknown:	  return 0;
	case VertexLayoutID::PUV:		  return 0;
	case VertexLayoutID::PNU:		  return 0;
	case VertexLayoutID::PNUTan:	  return 0;
	case VertexLayoutID::PNUTanSkin:  return 0;
	default:                          return 0;
	}
}


HRESULT Mesh::BuildPrimitive(const MeshMeta& meta, vector<uint8_t>& outVtx, vector<uint8_t>& outIdx,
	_uint& outVtxCount, _uint& outIdxCount, _uint& outVtxStride, DXGI_FORMAT& outIdxFmt) const
{
	if (meta.layout != VertexLayoutID::PNUTan)
		return E_INVALIDARG;

	// PNUTan: pos(12) + normal(12) + uv(8) + tangent(16) = 48bytes
	outVtxStride = 48u;
	constexpr _uint POS_OFFSET     = 0u;
	constexpr _uint NORMAL_OFFSET  = 12u;
	constexpr _uint UV_OFFSET      = 24u;
	constexpr _uint TANGENT_OFFSET = 32u;

	auto writeVertex = [&](uint8_t* dst, _float3 pos, _float3 normal, _float2 uv, _float4 tangent)
		{
			memset(dst, 0, outVtxStride);
			*reinterpret_cast<_float3*>(dst + POS_OFFSET)     = pos;
			*reinterpret_cast<_float3*>(dst + NORMAL_OFFSET)  = normal;
			*reinterpret_cast<_float2*>(dst + UV_OFFSET)      = uv;
			*reinterpret_cast<_float4*>(dst + TANGENT_OFFSET) = tangent;
		};

	auto setIdxFormat = [&](_uint vcount) { outIdxFmt = (vcount <= 65535) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT; };

	switch (meta.primitive)
	{
	case PRIMITIVE::Quad:
	{
		const float halfX = meta.sizeX * 0.5f;
		const float halfY = meta.sizeY * 0.5f;

		outVtxCount = 4;
		outIdxCount = 6;
		setIdxFormat(outVtxCount);

		outVtx.resize(outVtxCount * outVtxStride);
		outIdx.resize(outIdxCount * ((outIdxFmt == DXGI_FORMAT_R16_UINT) ? 2 : 4));

		const _float3 normal  = { 0, 0, 1 };
		const _float4 tangent = { 1, 0, 0, 1 };
		const _float3 pos[4]  = { { -halfX, -halfY, 0 }, { halfX, -halfY, 0 }, { halfX, halfY, 0 }, { -halfX, halfY, 0 } };
		const _float2 uv[4]   = { { 0, 1 }, { 1, 1 }, { 1, 0 }, { 0, 0 } };

		for (_uint i = 0; i < 4; ++i)
			writeVertex(outVtx.data() + i * outVtxStride, pos[i], normal, uv[i], tangent);

		const uint16_t idx16[6] = { 0, 1, 2,  0, 2, 3 };
		if (outIdxFmt == DXGI_FORMAT_R16_UINT)
			memcpy(outIdx.data(), idx16, sizeof(idx16));
		else
		{
			_uint* data = reinterpret_cast<_uint*>(outIdx.data());
			for (int i = 0; i < 6; ++i)
				data[i] = idx16[i];
		}
		return S_OK;
	}

	case PRIMITIVE::Cube:
	{
		const float halfX = meta.sizeX * 0.5f;
		const float halfY = meta.sizeY * 0.5f;
		const float halfZ = meta.sizeZ * 0.5f;

		outVtxCount = 24;
		outIdxCount = 36;
		setIdxFormat(outVtxCount);

		outVtx.resize(outVtxCount * outVtxStride);
		outIdx.resize(outIdxCount * ((outIdxFmt == DXGI_FORMAT_R16_UINT) ? 2 : 4));

		struct Face 
		{
			_float3 normal;
			_float4 tangent;
			_float3 v0, v1, v2, v3;
		};

		const Face faces[6] =
		{
			{ { 1, 0, 0 }, { 0, 0, -1, 1 }, { halfX, -halfY, -halfZ }, { halfX, -halfY, halfZ }, { halfX, halfY, halfZ }, { halfX, halfY, -halfZ } },
			// -X
			{ { -1, 0, 0 }, { 0, 0, 1, 1 }, { -halfX, -halfY, halfZ }, { -halfX, -halfY, -halfZ }, { -halfX, halfY, -halfZ }, { -halfX, halfY, halfZ } },
			// +Y
			{ { 0, 1, 0 }, { 1, 0, 0, 1 }, { -halfX, halfY, -halfZ }, { halfX, halfY, -halfZ }, { halfX, halfY, halfZ }, { -halfX, halfY, halfZ } },
			// -Y
			{ { 0, -1, 0 }, { -1, 0, 0, 1 }, { -halfX, -halfY, halfZ }, { halfX, -halfY, halfZ }, { halfX, -halfY, -halfZ }, { -halfX, -halfY, -halfZ } },
			// +Z
			{ { 0, 0, 1 }, { 1, 0, 0, 1 }, { -halfX, -halfY, halfZ }, { halfX, -halfY, halfZ }, { halfX, halfY, halfZ }, { -halfX, halfY, halfZ } },
			// -Z
			{ { 0, 0, -1 }, { -1, 0, 0, 1 }, { halfX, -halfY, -halfZ }, { -halfX, -halfY, -halfZ }, { -halfX, halfY, -halfZ }, { halfX, halfY, -halfZ } },
		};
		const _float2 UVs[4] = { { 0, 1 }, { 1, 1 }, { 1, 0 }, { 0, 0 } };

		// 24개 정점
		for (int i = 0; i < 6; ++i)
		{
			const auto& face = faces[i];
			const _float3 vertex[4] = { face.v0, face.v1, face.v2, face.v3 };
			for (int j = 0; j < 4; ++j)
			{
				const _float2 uv = UVs[j];
				writeVertex(outVtx.data() + (i * 4 + j) * outVtxStride, vertex[i], face.normal, uv, face.tangent);
			}
		}

		// 각 면 (0,1,2  0,2,3)
		auto emitFace = [&](int baseVtx, uint16_t* indice16, uint32_t* indice32, int& cursor)
			{
				const uint16_t quad[6] = { 0, 1, 2, 0, 2, 3 };
				for (int i = 0; i < 6; ++i)
				{
					const _uint idx = baseVtx + quad[i];
					if (outIdxFmt == DXGI_FORMAT_R16_UINT)
						indice16[cursor++] = (uint16_t)idx;
					else
						indice32[cursor++] = idx;
				}
			};

		if (outIdxFmt == DXGI_FORMAT_R16_UINT)
		{
			auto indice16 = reinterpret_cast<uint16_t*>(outIdx.size());
			int cur = 0;
			for (int i = 0; i < 6; ++i)
				emitFace(i * 4, indice16, nullptr, cur);
		}
		else
		{
			auto indice32 = reinterpret_cast<uint32_t*>(outIdx.data());
			int cur = 0;
			for (int i = 0; i < 6; ++i)
				emitFace(i * 4, nullptr, indice32, cur);
		}
		return S_OK;
	}

	case PRIMITIVE::Sphere:
	case PRIMITIVE::Cylinder:
	case PRIMITIVE::Capsule:
		return E_NOTIMPL;

	case PRIMITIVE::None:
	default:
		return E_INVALIDARG;

	}
}