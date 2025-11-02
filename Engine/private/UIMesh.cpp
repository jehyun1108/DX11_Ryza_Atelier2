#include "Enginepch.h"

HRESULT UIMesh::Create(ID3D11Device* device, size_t maxQuads)
{
    Destroy();
    this->maxQuads = maxQuads; 

    {
        D3D11_BUFFER_DESC vb{};
        vb.ByteWidth      = static_cast<UINT>(sizeof(UIVertex) * 4 * maxQuads);
        vb.Usage          = D3D11_USAGE_DYNAMIC;
        vb.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
        vb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        HR(device->CreateBuffer(&vb, nullptr, vbDynamic.GetAddressOf()));
    }

    {
        vector<uint16_t> indices(6 * maxQuads);
        for (size_t quadIndex = 0; quadIndex < maxQuads; ++quadIndex)
        {
            const uint16_t baseVertex = static_cast<uint16_t>(quadIndex * 4);
            const size_t   baseIndex = quadIndex * 6;

            indices[baseIndex + 0] = baseVertex + 0;
            indices[baseIndex + 1] = baseVertex + 1;
            indices[baseIndex + 2] = baseVertex + 2;
            indices[baseIndex + 3] = baseVertex + 0;
            indices[baseIndex + 4] = baseVertex + 2;
            indices[baseIndex + 5] = baseVertex + 3;
        }

        D3D11_BUFFER_DESC ib{};
        ib.ByteWidth = static_cast<UINT>(indices.size() * sizeof(uint16_t));
        ib.Usage = D3D11_USAGE_IMMUTABLE;
        ib.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA sd{};
        sd.pSysMem = indices.data();

        HR(device->CreateBuffer(&ib, &sd, ibStatic.GetAddressOf()));
    }

    return S_OK;
}

void UIMesh::Destroy()
{
    vbDynamic.Reset();
    ibStatic.Reset();
    maxQuads = 0;
}

void UIMesh::Bind(ID3D11DeviceContext* context, Shader& uiShader, CBuffer& uiCBuffer, const UICB& uiCB)
{
    uiShader.Bind(context);
    uiCBuffer.UpdateAndBind(uiCB, SHADER::VS, CBUFFERSLOT::UI);

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    UINT stride = sizeof(UIVertex);
    UINT offset = 0;
    ID3D11Buffer* vb = vbDynamic.Get();
    context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
    context->IASetIndexBuffer(ibStatic.Get(), DXGI_FORMAT_R16_UINT, 0);
}

void UIMesh::Draw(ID3D11DeviceContext* context, const vector<UIDrawItem>& items, ResolveTexture resolveTexture)
{
    if (items.empty()) return;

    // 정렬: zOrder → texKey → scissor (상태 변경 최소화)
    vector<const UIDrawItem*> sorted(items.size());
    for (size_t i = 0; i < items.size(); ++i)
        sorted[i] = &items[i];

    sort(sorted.begin(), sorted.end(),
        [](const UIDrawItem* a, const UIDrawItem* b)
        {
            if (a->zOrder != b->zOrder) return a->zOrder < b->zOrder;
            if (a->texKey != b->texKey) return a->texKey < b->texKey;

            if (a->useScissor != b->useScissor) return a->useScissor < b->useScissor;
            if (!a->useScissor) return false;

            if (a->scissorRect.x != b->scissorRect.x)           return a->scissorRect.x      < b->scissorRect.x;
            if (a->scissorRect.y != b->scissorRect.y)           return a->scissorRect.y      < b->scissorRect.y;
            if (a->scissorRect.width != b->scissorRect.width)   return a->scissorRect.width  < b->scissorRect.width;
            if (a->scissorRect.height != b->scissorRect.height) return a->scissorRect.height < b->scissorRect.height;
            return false;
        });

    const size_t quadCount = min(sorted.size(), maxQuads);
    assert(sorted.size() <= maxQuads && "UIMesh overflow: increase maxQuads");

    FillVB(context, sorted, quadCount);
    IssueBatches(context, sorted, resolveTexture, quadCount);
}

size_t UIMesh::FillVB(ID3D11DeviceContext* context, const vector<const UIDrawItem*>& sorted, size_t quadCount)
{
    D3D11_MAPPED_SUBRESOURCE mapped{};
    HR(context->Map(vbDynamic.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));

    auto* vertexDst = reinterpret_cast<UIVertex*>(mapped.pData);

    // UV는 항상 0~1 (전체 텍스처)
    constexpr float u0 = 0.f, v0 = 0.f, u1 = 1.f, v1 = 1.f;

    for (size_t quadIndex = 0; quadIndex < quadCount; ++quadIndex)
    {
        const UIDrawItem& item = *sorted[quadIndex];

        const float left = item.dstRect.x;
        const float top = item.dstRect.y;
        const float right = item.dstRect.x + item.dstRect.width;
        const float bottom = item.dstRect.y + item.dstRect.height;

        // CCW 사각형(두 개의 삼각형)
        vertexDst[0] = { left,  top,    u0, v0 };
        vertexDst[1] = { right, top,    u1, v0 };
        vertexDst[2] = { right, bottom, u1, v1 };
        vertexDst[3] = { left,  bottom, u0, v1 };

        vertexDst += 4;
    }

    context->Unmap(vbDynamic.Get(), 0);
    return quadCount;
}

void UIMesh::IssueBatches(ID3D11DeviceContext* context, const vector<const UIDrawItem*>& sorted, ResolveTexture resolveTexture, size_t quadCount)
{
    size_t runStart = 0;

    bool   scissorActive = false;
    UIRect curScissor{};

    auto ApplyScissorIfNeeded = [&](const UIDrawItem& item)
        {
            if (!item.useScissor) return;
            if (!scissorActive || !IsSameScissor(item, UIDrawItem{ .useScissor = true, .scissorRect = curScissor }))
            {
                const LONG left = static_cast<LONG>(floor(item.scissorRect.x));
                const LONG top = static_cast<LONG>(floor(item.scissorRect.y));
                const LONG right = static_cast<LONG>(ceil(item.scissorRect.x + item.scissorRect.width));
                const LONG bottom = static_cast<LONG>(ceil(item.scissorRect.y + item.scissorRect.height));
                const D3D11_RECT rect = { left, top, right, bottom };
                context->RSSetScissorRects(1, &rect);

                scissorActive = true;
                curScissor = item.scissorRect;
            }
        };

    while (runStart < quadCount)
    {
        const UIDrawItem& first = *sorted[runStart];
        const Texture* texture = resolveTexture(first.texKey);
        assert(texture && "Texture missing!");

        ApplyScissorIfNeeded(first);

        // 같은 텍스처 + 같은 시저를 하나의 드로우로 묶기
        size_t runEnd = runStart + 1;
        while (runEnd < quadCount)
        {
            const UIDrawItem& cur = *sorted[runEnd];
            if (cur.texKey != first.texKey) break;
            if (!IsSameScissor(cur, first)) break;
            ++runEnd;
        }

        ID3D11ShaderResourceView* srv = texture->GetSrv();
        context->PSSetShaderResources(0, 1, &srv);

        const UINT indexCount = static_cast<UINT>((runEnd - runStart) * 6);
        const UINT startIndex = static_cast<UINT>(runStart * 6);
        context->DrawIndexed(indexCount, startIndex, 0);

        ID3D11ShaderResourceView* nullSrv[1] = { nullptr };
        context->PSSetShaderResources(0, 1, nullSrv);

        runStart = runEnd;
    }
}
