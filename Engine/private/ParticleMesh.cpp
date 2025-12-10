#include "Enginepch.h"
#include "ParticleMesh.h"

void ParticleMesh::Create(ID3D11Device* device, int maxInstances)
{
	this->maxInstances = maxInstances;
    // ================== VB =========================================
	ParticleVertex vertices[4];

    vertices[0].localPos = { -0.5f, -0.5f, 0.0f };
    vertices[0].uv       = { 0.0f,  1.0f };

    vertices[1].localPos = { 0.5f, -0.5f, 0.0f }; 
    vertices[1].uv       = { 1.0f,  1.0f };

    vertices[2].localPos = { -0.5f, 0.5f, 0.0f }; 
    vertices[2].uv       = { 0.0f,  0.0f };

    vertices[3].localPos = { 0.5f,  0.5f, 0.0f }; 
    vertices[3].uv       = { 1.0f,  0.0f };

    D3D11_BUFFER_DESC vbDesc{};
    vbDesc.Usage               = D3D11_USAGE_IMMUTABLE;
    vbDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.ByteWidth           = sizeof(vertices);
    vbDesc.StructureByteStride = sizeof(ParticleVertex);

    D3D11_SUBRESOURCE_DATA vbData{};
    vbData.pSysMem = vertices;

    HR(device->CreateBuffer(&vbDesc, &vbData, quadVB.GetAddressOf()));
    // ================ IB =============================================
    uint16_t indices[6] = {
        0, 1, 2,
        2, 1, 3
    };

    D3D11_BUFFER_DESC ibDesc{};
    ibDesc.Usage               = D3D11_USAGE_IMMUTABLE;
    ibDesc.BindFlags           = D3D11_BIND_INDEX_BUFFER;
    ibDesc.ByteWidth           = sizeof(indices);
    ibDesc.StructureByteStride = sizeof(uint16_t);

    D3D11_SUBRESOURCE_DATA ibData{};
    ibData.pSysMem = indices;

    HR(device->CreateBuffer(&ibDesc, &ibData, quadIB.GetAddressOf()));
    // ============== InstanceBuffer =====================================
    D3D11_BUFFER_DESC instDesc{};
    instDesc.Usage               = D3D11_USAGE_DYNAMIC;
    instDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
    instDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
    instDesc.ByteWidth           = static_cast<UINT>(sizeof(ParticleInstance) * maxInstances);
    instDesc.StructureByteStride = sizeof(ParticleInstance);

    HR(device->CreateBuffer(&instDesc, nullptr, instanceVB.GetAddressOf()));
}

void ParticleMesh::Draw(ID3D11DeviceContext* ctx, const vector<const ParticleDrawItem*>& items)
{
    if (items.empty()) return;

    const int instanceCount = static_cast<int>(min<size_t>(items.size(), static_cast<size_t>(maxInstances)));

    D3D11_MAPPED_SUBRESOURCE mapped{};
    HR(ctx->Map(instanceVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));

    auto* dst = static_cast<ParticleInstance*>(mapped.pData);
    for (int i = 0; i < instanceCount; ++i)
    {
        const ParticleDrawItem* src = items[static_cast<size_t>(i)];
        ParticleInstance& inst = dst[i];
        inst.pos    = src->pos;
        inst.size   = src->size;
        inst.color  = src->color;
        inst.rotRad = src->rotRad;
        inst.uvMin  = src->uvMin; 
        inst.uvMax  = src->uvMax;
    }
    ctx->Unmap(instanceVB.Get(), 0);

    ID3D11Buffer* bufs[2] = { quadVB.Get(), instanceVB.Get() };

    UINT strides[2] = {
        sizeof(ParticleVertex),
        sizeof(ParticleInstance)
    };

    UINT offsets[2] = { 0, 0 };

    ctx->IASetVertexBuffers(0, 2, bufs, strides, offsets);
    ctx->IASetIndexBuffer(quadIB.Get(), DXGI_FORMAT_R16_UINT, 0);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->DrawIndexedInstanced(6, instanceCount, 0, 0, 0);
}