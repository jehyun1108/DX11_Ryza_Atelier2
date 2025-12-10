#include "Enginepch.h"
#include "EffectSystem.h"

inline static bool IsSameScissor(const UIDrawItem& a, const UIDrawItem& b)
{
    if (a.useScissor != b.useScissor) return false;
    if (!a.useScissor) return true;
    return a.scissorRect.x == b.scissorRect.x &&
        a.scissorRect.y == b.scissorRect.y &&
        a.scissorRect.width == b.scissorRect.width &&
        a.scissorRect.height == b.scissorRect.height;
}
// ===================================================================================================
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
        assert(ibStatic);
    }
    return S_OK;
}

void UIMesh::Destroy()
{
    vbDynamic.Reset();
    ibStatic.Reset();
    maxQuads = 0;
}

void UIMesh::Bind(ID3D11DeviceContext* context, CBuffer& uiCBuffer, const UICB& uiCB)
{
    uiCBuffer.UpdateAndBind(uiCB, SHADER::VS | SHADER::PS, CBUFFERSLOT::UI);

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

    vector<const UIDrawItem*> sorted(items.size());
    for (size_t i = 0; i < items.size(); ++i) sorted[i] = &items[i];

    sort(sorted.begin(), sorted.end(), [](const UIDrawItem* a, const UIDrawItem* b)
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
    assert(quadCount <= maxQuads);

    D3D11_MAPPED_SUBRESOURCE mapped{};
    HR(context->Map(vbDynamic.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
    auto* dst = reinterpret_cast<UIVertex*>(mapped.pData);

    for (size_t i = 0; i < quadCount; ++i)
    {
        const UIDrawItem& it = *sorted[i];

        const float x = it.dstRect.x;
        const float y = it.dstRect.y;
        const float w = it.dstRect.width;
        const float h = it.dstRect.height;

        const float px = x + it.pivotNX * w;
        const float py = y + it.pivotNY * h;

        const float rad = it.rotDeg * 0.01745329251994329577f;
        const float c = cosf(rad);
        const float s = sinf(rad);

        float x0 = x;       float y0 = y;
        float x1 = x + w;   float y1 = y;
        float x2 = x + w;   float y2 = y + h;
        float x3 = x;       float y3 = y + h;

        auto rot = [&](float ox, float oy) -> pair<float, float>
            {
                const float dx = ox - px;
                const float dy = oy - py;
                return { px + dx * c - dy * s, py + dx * s + dy * c };
            };

        auto [rx0, ry0] = rot(x0, y0);
        auto [rx1, ry1] = rot(x1, y1);
        auto [rx2, ry2] = rot(x2, y2);
        auto [rx3, ry3] = rot(x3, y3);

        float uLeft  = it.srcU0;
        float uRight = it.srcU1;
        float vTop   = it.srcV0;
        float vBot   = it.srcV1;

        float u00 = uLeft,  v00 = vTop; // LT
        float u10 = uRight, v10 = vTop; // RT
        float u11 = uRight, v11 = vBot; // RB
        float u01 = uLeft,  v01 = vBot; // LB

        switch (it.flipMode)
        {
        case UIFlipMode::None:   break;
        case UIFlipMode::FlipX:  swap(u00, u10); swap(u01, u11); break;
        case UIFlipMode::FlipY:  swap(v00, v01); swap(v10, v11); break;
        case UIFlipMode::FlipXY: swap(u00, u10); swap(u01, u11); swap(v00, v01); swap(v10, v11); break;
        }

        const float fillX = it.fillRatioX;
        const float fillY = it.fillRatioY;
        const float fillMode = (it.fillMode == UIFillMode::RingCW) ? 1.f : 0.f;
        const float alpha = it.alpha;
        const float mask = (it.maskType == UIMaskType::Circle) ? 1.f : 0.f;
        const _float4 color = it.color;

        // v0 (LT)
        dst[0].posX     = rx0;
        dst[0].posY     = ry0;
        dst[0].uvX      = u00;
        dst[0].uvY      = v00;
        dst[0].fillX    = fillX;
        dst[0].fillY    = fillY;
        dst[0].mode     = fillMode;
        dst[0].alpha    = alpha;
        dst[0].maskType = mask;
        dst[0].color    = color;

        // v1 (RT)
        dst[1].posX     = rx1;
        dst[1].posY     = ry1;
        dst[1].uvX      = u10;
        dst[1].uvY      = v10;
        dst[1].fillX    = fillX;
        dst[1].fillY    = fillY;
        dst[1].mode     = fillMode;
        dst[1].alpha    = alpha;
        dst[1].maskType = mask;
        dst[1].color    = color;
        // v2 (RB)
        dst[2].posX     = rx2;
        dst[2].posY     = ry2;
        dst[2].uvX      = u11;
        dst[2].uvY      = v11;
        dst[2].fillX    = fillX;
        dst[2].fillY    = fillY;
        dst[2].mode     = fillMode;
        dst[2].alpha    = alpha;
        dst[2].maskType = mask;
        dst[2].color    = color;
        // v3 (LB)
        dst[3].posX     = rx3;
        dst[3].posY     = ry3;
        dst[3].uvX      = u01;
        dst[3].uvY      = v01;
        dst[3].fillX    = fillX;
        dst[3].fillY    = fillY;
        dst[3].mode     = fillMode;
        dst[3].alpha    = alpha;
        dst[3].maskType = mask;
        dst[3].color    = color;

        dst += 4;
    }

    context->Unmap(vbDynamic.Get(), 0);
    return quadCount;
}
void UIMesh::IssueBatches(ID3D11DeviceContext* context, const vector<const UIDrawItem*>& sorted, ResolveTexture resolveTexture, size_t quadCount)
{
    assert(quadCount <= maxQuads);
    assert(quadCount <= sorted.size());

    size_t runStart = 0;
    bool   scissorActive = false;
    UIRect curScissor{};

    D3D11_VIEWPORT vp{};
    UINT numVP = 1;
    context->RSGetViewports(&numVP, &vp);
    const D3D11_RECT fullRect = { 0, 0, (LONG)vp.Width, (LONG)vp.Height };

    auto SetFullScissor = [&]()
        {
            context->RSSetScissorRects(1, &fullRect);
            scissorActive = false;
        };

    auto ApplyScissor = [&](const UIDrawItem& item)
        {
            if (!item.useScissor)
            {
                if (scissorActive)
                    SetFullScissor();
                return;
            }

            if (!scissorActive ||
                curScissor.x != item.scissorRect.x ||
                curScissor.y != item.scissorRect.y ||
                curScissor.width != item.scissorRect.width ||
                curScissor.height != item.scissorRect.height)
            {
                const D3D11_RECT rect =
                {
                    (LONG)floor(item.scissorRect.x),
                    (LONG)floor(item.scissorRect.y),
                    (LONG)ceil(item.scissorRect.x + item.scissorRect.width),
                    (LONG)ceil(item.scissorRect.y + item.scissorRect.height)
                };
                context->RSSetScissorRects(1, &rect);
                scissorActive = true;
                curScissor = item.scissorRect;
            }
        };

    while (runStart < quadCount)
    {
        const UIDrawItem& first = *sorted[runStart];
        ID3D11ShaderResourceView* srv = resolveTexture(first.texKey);
        assert(srv && "srv missing");

        ApplyScissor(first);

        size_t runEnd = runStart + 1;
        while (runEnd < quadCount)
        {
            const UIDrawItem& cur = *sorted[runEnd];
            if (cur.texKey != first.texKey) break;
            if (!IsSameScissor(cur, first)) break;
            ++runEnd;
        }

        context->PSSetShaderResources(0, 1, &srv);

        const UINT indexCount = static_cast<UINT>((runEnd - runStart) * 6);
        const UINT startIndex = static_cast<UINT>(runStart * 6);
        context->DrawIndexed(indexCount, startIndex, 0);

        ID3D11ShaderResourceView* nullSrv[1] = { nullptr };
        context->PSSetShaderResources(0, 1, nullSrv);

        runStart = runEnd;
    }

    if (scissorActive)
        SetFullScissor();
}