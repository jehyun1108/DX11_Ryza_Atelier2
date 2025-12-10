#include "Enginepch.h"
#include "TrailMesh.h"

void TrailMesh::Create(ID3D11Device* device, int maxVertices, int maxIndices)
{
	this->maxVertices = maxVertices;
	this->maxIndices  = maxIndices;

	// ================= VB  ======================
	D3D11_BUFFER_DESC vbDesc{};
	vbDesc.Usage = D3D11_USAGE_DYNAMIC;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbDesc.ByteWidth = static_cast<UINT>(sizeof(TrailVertex) * maxVertices);
	vbDesc.StructureByteStride = sizeof(TrailVertex);

	HR(device->CreateBuffer(&vbDesc, nullptr, vb.GetAddressOf()));

	// ================= IB ==========
	const int maxQuadsByVertex = maxVertices / 4;
	const int maxQuadsByIndex = maxIndices / 6;
	const int quadCap = (min)(maxQuadsByVertex, maxQuadsByIndex);

	const int indexCount = quadCap * 6;
	vector<uint16_t> indices;
	indices.resize(static_cast<size_t>(indexCount));

	int idx = 0;
	for (int q = 0; q < quadCap; ++q)
	{
		const uint16_t vBase = static_cast<uint16_t>(q * 4);

		indices[idx++] = vBase + 0;
		indices[idx++] = vBase + 1;
		indices[idx++] = vBase + 2;

		indices[idx++] = vBase + 2;
		indices[idx++] = vBase + 1;
		indices[idx++] = vBase + 3;
	}

	this->maxIndices = indexCount;

	D3D11_BUFFER_DESC ibDesc{};
	ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.ByteWidth = static_cast<UINT>(sizeof(uint16_t) * indices.size());
	ibDesc.StructureByteStride = sizeof(uint16_t);

	D3D11_SUBRESOURCE_DATA ibData{};
	ibData.pSysMem = indices.data();

	HR(device->CreateBuffer(&ibDesc, &ibData, ib.GetAddressOf()));
}

void TrailMesh::Draw(ID3D11DeviceContext* ctx, const TrailSnapshot& snapshot, const CameraProxy& cam)
{
    if (snapshot.trails.empty()) return;

    int totalQuads = 0;
    for (const auto& trail : snapshot.trails)
    {
        int n = static_cast<int>(trail.points.size());
        if (n >= 2)
            totalQuads += (n - 1);
    }
    if (totalQuads <= 0)  return;

    const int vertexCount = totalQuads * 4;
    const int indexCount = totalQuads * 6;

    D3D11_MAPPED_SUBRESOURCE mapped{};
    HR(ctx->Map(vb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));

    auto* dst = static_cast<TrailVertex*>(mapped.pData);
    int   cursor = 0;

    _vec camF = XMLoadFloat4(&cam.camForward);
    camF = XMVector3Normalize(camF);

    vector<_float3> left;
    vector<_float3> right;
    vector<_float4> cols;
    vector<float>   vList;

    for (const auto& trail : snapshot.trails)
    {
        const TrailDesc* desc = trail.desc;
        const auto& pts = trail.points;

        const int n = static_cast<int>(pts.size());
        if (n < 2) continue;

        left.resize(n);
        right.resize(n);
        cols.resize(n);
        vList.resize(n);
        const float invSeg = (n > 1) ? 1.0f / static_cast<float>(n - 1) : 0.0f;

        for (int i = 0; i < n; ++i)
        {
            const TrailPointProxy& p = pts[static_cast<size_t>(i)];

            _float3 dir{};
            if (i == 0)
            {
                dir.x = pts[1].pos.x - pts[0].pos.x;
                dir.y = pts[1].pos.y - pts[0].pos.y;
                dir.z = pts[1].pos.z - pts[0].pos.z;
            }
            else if (i == n - 1)
            {
                dir.x = pts[n - 1].pos.x - pts[n - 2].pos.x;
                dir.y = pts[n - 1].pos.y - pts[n - 2].pos.y;
                dir.z = pts[n - 1].pos.z - pts[n - 2].pos.z;
            }
            else
            {
                dir.x = pts[i + 1].pos.x - pts[i - 1].pos.x;
                dir.y = pts[i + 1].pos.y - pts[i - 1].pos.y;
                dir.z = pts[i + 1].pos.z - pts[i - 1].pos.z;
            }

            _vec vDir = XMLoadFloat3(&dir);
            vDir = XMVector3Normalize(vDir);
            XMStoreFloat3(&dir, vDir);

            _vec vSide = XMVector3Normalize(XMVector3Cross(camF, vDir));
            _float3 side;
            XMStoreFloat3(&side, vSide);

            float wT = EffectUtility::Curve(desc->widthCurve, p.t);
            float aT = EffectUtility::Curve(desc->alphaCurve, p.t);

            float width = desc->widthStart + (desc->widthEnd - desc->widthStart) * wT;
            float half = width * 0.5f;

            _float4 c{};
            c.x = desc->colorStart.x + (desc->colorEnd.x - desc->colorStart.x) * p.t;
            c.y = desc->colorStart.y + (desc->colorEnd.y - desc->colorStart.y) * p.t;
            c.z = desc->colorStart.z + (desc->colorEnd.z - desc->colorStart.z) * p.t;
            c.w = desc->colorStart.w + (desc->colorEnd.w - desc->colorStart.w) * aT;

            _float3 leftPos{
                p.pos.x - side.x * half,
                p.pos.y - side.y * half,
                p.pos.z - side.z * half
            };
            _float3 rightPos{
                p.pos.x + side.x * half,
                p.pos.y + side.y * half,
                p.pos.z + side.z * half
            };

            left[i] = leftPos;
            right[i] = rightPos;
            cols[i] = c;
            vList[i] = static_cast<float>(i) * invSeg;
        }

        for (int i = 0; i < n - 1; ++i)
        {
            TrailVertex& v0p = dst[cursor + 0];
            TrailVertex& v1p = dst[cursor + 1];
            TrailVertex& v2p = dst[cursor + 2];
            TrailVertex& v3p = dst[cursor + 3];

            v0p.pos = left[i];
            v0p.color = cols[i];
            v0p.uv = _float2{ 0.f, vList[i] };

            v1p.pos = right[i];
            v1p.color = cols[i];
            v1p.uv = _float2{ 1.f, vList[i] };

            v2p.pos = left[i + 1];
            v2p.color = cols[i + 1];
            v2p.uv = _float2{ 0.f, vList[i + 1] };

            v3p.pos = right[i + 1];
            v3p.color = cols[i + 1];
            v3p.uv = _float2{ 1.f, vList[i + 1] };

            cursor += 4;
        }
    }

    ctx->Unmap(vb.Get(), 0);

    ID3D11Buffer* vbuf = vb.Get();
    const _uint stride = sizeof(TrailVertex);
    const _uint offset = 0;

    ctx->IASetVertexBuffers(0, 1, &vbuf, &stride, &offset);
    ctx->IASetIndexBuffer(ib.Get(), DXGI_FORMAT_R16_UINT, 0);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->DrawIndexed(static_cast<_uint>(indexCount), 0, 0);
}