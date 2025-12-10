#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL DebugUtility
{
public:
    inline static void AssertIAState_FullscreenTri(ID3D11DeviceContext* ctx)
    {
        ID3D11InputLayout* il = nullptr;
        ctx->IAGetInputLayout(&il);
        assert(il == nullptr);
        if (il) il->Release();

        D3D11_PRIMITIVE_TOPOLOGY topo{};
        ctx->IAGetPrimitiveTopology(&topo);
        assert(topo == D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        ID3D11Buffer* vb = nullptr;
        UINT stride = 0, offset = 0;
        ctx->IAGetVertexBuffers(0, 1, &vb, &stride, &offset);
        assert(vb == nullptr);
        if (vb) vb->Release();

        ID3D11Buffer* ib = nullptr;
        DXGI_FORMAT fmt{};
        UINT ofs{};
        ctx->IAGetIndexBuffer(&ib, &fmt, &ofs);
        assert(ib == nullptr && fmt == DXGI_FORMAT_UNKNOWN);
        if (ib) ib->Release();
    }

    inline static void AssertRTVDSV(ID3D11DeviceContext* ctx, ID3D11RenderTargetView* expectRTV,  ID3D11DepthStencilView* expectDSV)
    {
        ID3D11RenderTargetView* rtvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
        ID3D11DepthStencilView* dsv = nullptr;
        ctx->OMGetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, rtvs, &dsv);

        assert(rtvs[0] == expectRTV);
        assert(dsv == expectDSV);

        for (auto* r : rtvs) if (r) r->Release();
        if (dsv) dsv->Release();
    }

    inline static void AssertBlend_Additive(ID3D11DeviceContext* ctx)
    {
        ID3D11BlendState* bs = nullptr;
        FLOAT factor[4]; UINT mask{};
        ctx->OMGetBlendState(&bs, factor, &mask);
        assert(bs != nullptr);
        D3D11_BLEND_DESC desc{}; bs->GetDesc(&desc);
        const auto& rt0 = desc.RenderTarget[0];
        assert(rt0.BlendEnable == TRUE);
        assert(rt0.SrcBlend == D3D11_BLEND_ONE);
        assert(rt0.DestBlend == D3D11_BLEND_ONE);
        assert(rt0.BlendOp == D3D11_BLEND_OP_ADD);
        bs->Release();
    }

    inline static void AssertBlend_Opaque(ID3D11DeviceContext* ctx)
    {
        ID3D11BlendState* bs = nullptr;
        FLOAT factor[4]; UINT mask{};
        ctx->OMGetBlendState(&bs, factor, &mask);
        assert(bs != nullptr);
        D3D11_BLEND_DESC desc{}; bs->GetDesc(&desc);
        const auto& rt0 = desc.RenderTarget[0];
        assert(rt0.BlendEnable == FALSE);
        bs->Release();
    }

    inline static void AssertDepth_Off(ID3D11DeviceContext* ctx)
    {
        ID3D11DepthStencilState* ds = nullptr; UINT ref{};
        ctx->OMGetDepthStencilState(&ds, &ref);
        assert(ds != nullptr);
        D3D11_DEPTH_STENCIL_DESC d{}; ds->GetDesc(&d);
        assert(d.DepthEnable == FALSE);
        ds->Release();
    }

    inline static void AssertDepth_ReadOnly(ID3D11DeviceContext* ctx)
    {
        ID3D11DepthStencilState* ds = nullptr; UINT ref{};
        ctx->OMGetDepthStencilState(&ds, &ref);
        assert(ds != nullptr);
        D3D11_DEPTH_STENCIL_DESC d{}; ds->GetDesc(&d);
        // 읽기 전용 = 테스트 On, Write Off
        assert(d.DepthEnable == TRUE);
        assert(d.DepthWriteMask == D3D11_DEPTH_WRITE_MASK_ZERO);
        ds->Release();
    }

    inline static void AssertPS_SRV_Equals(ID3D11DeviceContext* ctx, UINT slot, ID3D11ShaderResourceView* expect)
    {
        ID3D11ShaderResourceView* s = nullptr;
        ctx->PSGetShaderResources(slot, 1, &s);
        assert(s == expect);
        if (s) s->Release();
    }

    //inline static void AssertAllStages_SRVsCleared(ID3D11DeviceContext* ctx, UINT startSlot, UINT count)
    //{
    //    // VS/HS/DS/GS/PS/CS 전부 확인
    //    auto check = [&](auto Getter)
    //        {
    //            vector<ID3D11ShaderResourceView*> tmp(count, nullptr);
    //            Getter(ctx, startSlot, count, tmp.data());
    //            for (auto* p : tmp) assert(p == nullptr), void(p ? p->Release() : (void)0);
    //        };
    //    check([](auto* c, UINT s, UINT n, ID3D11ShaderResourceView** o) { c->VSGetShaderResources(s, n, o); });
    //    check([](auto* c, UINT s, UINT n, ID3D11ShaderResourceView** o) { c->HSGetShaderResources(s, n, o); });
    //    check([](auto* c, UINT s, UINT n, ID3D11ShaderResourceView** o) { c->DSGetShaderResources(s, n, o); });
    //    check([](auto* c, UINT s, UINT n, ID3D11ShaderResourceView** o) { c->GSGetShaderResources(s, n, o); });
    //    check([](auto* c, UINT s, UINT n, ID3D11ShaderResourceView** o) { c->PSGetShaderResources(s, n, o); });
    //    check([](auto* c, UINT s, UINT n, ID3D11ShaderResourceView** o) { c->CSGetShaderResources(s, n, o); });
    //}

    inline static void DebugAssert_GeometryDepthWritable(ID3D11DeviceContext* ctx,
        ID3D11DepthStencilView* expectedDSV,
        ID3D11DepthStencilState* expectedDSS,
        const D3D11_VIEWPORT& expectedVP)
    {
        ID3D11RenderTargetView* rtvs[8] = {};
        ID3D11DepthStencilView* dsv = nullptr;
        ctx->OMGetRenderTargets(8, rtvs, &dsv);
        assert(dsv == expectedDSV);

        ID3D11DepthStencilState* curDSS = nullptr;
        UINT stencilRef = 0;
        ctx->OMGetDepthStencilState(&curDSS, &stencilRef);
        assert(curDSS == expectedDSS);
        if (curDSS) curDSS->Release();

        UINT vpCount = 16;
        D3D11_VIEWPORT vps[16] = {};
        ctx->RSGetViewports(&vpCount, vps);
        assert(vpCount == 1);
        assert(fabs(vps[0].Width - expectedVP.Width) < 0.5f);
        assert(fabs(vps[0].Height - expectedVP.Height) < 0.5f);

        if (dsv) dsv->Release();
        for (auto* r : rtvs) if (r) r->Release();
    }

    inline static void DebugAssert_NoSRVBound(ID3D11DeviceContext* ctx, ID3D11ShaderResourceView* forbid)
    {
        ID3D11ShaderResourceView* srvs[16] = {};
        ctx->PSGetShaderResources(0, 16, srvs);
        for (int i = 0; i < 16; ++i) { assert(srvs[i] != forbid); if (srvs[i]) srvs[i]->Release(); }

        ctx->VSGetShaderResources(0, 16, srvs);
        for (int i = 0; i < 16; ++i) { assert(srvs[i] != forbid); if (srvs[i]) srvs[i]->Release(); }
    }
};

NS_END