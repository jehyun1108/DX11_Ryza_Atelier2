#include "Enginepch.h"

#include "DebugUtility.h"
#include "RenderTargetMinimap.h"

// ==============================================================================================
RenderTargetSystem::RenderTargetSystem(SystemRegistry& registry) : registry(registry), game(GAME)
{
    device = game.GetDevicePtr();
    dev    = game.GetDevice();
    ctx    = game.GetContext();
}

void RenderTargetSystem::Init(_uint w, _uint h, const GBufferSpec& s)
{
    width = w; height = h; spec = s;
    CreateGBuffer();
    CreateLightAccum();
    CreateDepth();
    InitDressing(1440, 1440);
}

void RenderTargetSystem::Resize(_uint w, _uint h)
{
    width = w; height = h;
    Destroy();
    CreateGBuffer();
    CreateLightAccum();
    CreateDepth();
}

void RenderTargetSystem::Destroy()
{
    ctx->OMSetRenderTargets(0, nullptr, nullptr);
    UnbindAllStageSRVs(0, 16);

    for (size_t i = 0; i < gbufferCount; ++i)
    {
        gbuffer[i].srv.Reset();
        gbuffer[i].rtv.Reset();
        gbuffer[i].tex.Reset();
        gbuffer[i].fmt = DXGI_FORMAT_UNKNOWN;
    }
    lightAccum.srv.Reset();
    lightAccum.rtv.Reset();
    lightAccum.tex.Reset();
    lightAccum.fmt = DXGI_FORMAT_UNKNOWN;

    depth.srv.Reset();
    depth.dsv.Reset();
    depth.tex.Reset();
    depth.texFmt = DXGI_FORMAT_R32_TYPELESS;
}

void RenderTargetSystem::InitDressing(_uint w, _uint h)
{
    dressingWidth = w;
    dressingHeight = h;
    // Color RT
    {
        DXGI_FORMAT fmt = DXGI_FORMAT_R16G16B16A16_FLOAT;
        auto tex = device->CreateTex2D(
            dressingWidth, dressingHeight, fmt,
            D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);

        auto rtv = device->CreateRTV(tex.Get());
        auto srv = device->CreateSRV(tex.Get());

        dressingColor.tex = tex;
        dressingColor.rtv = rtv;
        dressingColor.srv = srv;
        dressingColor.fmt = fmt;
    }
    // Depth
    {
        DXGI_FORMAT texFmt = DXGI_FORMAT_R32_TYPELESS;
        DXGI_FORMAT dsvFmt = DXGI_FORMAT_D32_FLOAT;
        DXGI_FORMAT srvFmt = DXGI_FORMAT_R32_FLOAT;

        auto tex = device->CreateTex2D(
            dressingWidth, dressingHeight, texFmt,
            D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
        dsvDesc.Format = dsvFmt;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;

        ComPtr<ID3D11DepthStencilView> dsv;
        dev->CreateDepthStencilView(tex.Get(), &dsvDesc, dsv.GetAddressOf());
        dressingDepth.tex = tex;
        dressingDepth.dsv = dsv;
        dressingDepth.texFmt = texFmt;
    }
}

void RenderTargetSystem::BeginDressingPass(bool clear)
{
    UINT count = 1;
    ctx->RSGetViewports(&count, &dressingPrevVP);
    dressingPrevSaved = true;

    D3D11_VIEWPORT vp{};
    vp.TopLeftX = 0.f;
    vp.TopLeftY = 0.f;
    vp.Width = static_cast<float>(dressingWidth);
    vp.Height = static_cast<float>(dressingHeight);
    vp.MinDepth = 0.f;
    vp.MaxDepth = 1.f;
    ctx->RSSetViewports(1, &vp);

    ID3D11RenderTargetView* rtv = dressingColor.rtv.Get();
    ID3D11DepthStencilView* dsv = dressingDepth.dsv.Get();
    ctx->OMSetRenderTargets(1, &rtv, dsv);

    if (clear)
    {
        const float clearCol[4] = { 0, 0, 0, 0 };
        ctx->ClearRenderTargetView(rtv, clearCol);
        ctx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.f, 0);
    }
}

void RenderTargetSystem::EndDressingPass()
{
    ctx->OMSetRenderTargets(0, nullptr, nullptr);

    if (dressingPrevSaved)
    {
        ctx->RSSetViewports(1, &dressingPrevVP);
        dressingPrevSaved = false;
    }
}

void RenderTargetSystem::CreateGBuffer()
{
    for (size_t i = 0; i < gbufferCount; ++i)
    {
        const DXGI_FORMAT fmt = PickFormatBySlot(spec, static_cast<GBufferSlot>(i));
        auto tex = device->CreateTex2D(width, height, fmt, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
        auto rtv = device->CreateRTV(tex.Get());
        auto srv = device->CreateSRV(tex.Get());
        gbuffer[i].tex = tex; gbuffer[i].rtv = rtv; gbuffer[i].srv = srv; gbuffer[i].fmt = fmt;
    }
}

void RenderTargetSystem::CreateLightAccum()
{
    const DXGI_FORMAT fmt = spec.lightAccum;
    auto tex = device->CreateTex2D(width, height, fmt, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
    auto rtv = device->CreateRTV(tex.Get());
    auto srv = device->CreateSRV(tex.Get());
    lightAccum.tex = tex; lightAccum.rtv = rtv; lightAccum.srv = srv; lightAccum.fmt = fmt;
}

void RenderTargetSystem::CreateDepth()
{
    DXGI_FORMAT texFmt = DXGI_FORMAT_R32_TYPELESS;
    DXGI_FORMAT dsvFmt = DXGI_FORMAT_D32_FLOAT;
    DXGI_FORMAT srvFmt = DXGI_FORMAT_R32_FLOAT;

    auto tex = device->CreateTex2D(width, height, texFmt, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format             = dsvFmt;
    dsvDesc.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    ComPtr<ID3D11DepthStencilView> dsv;
    dev->CreateDepthStencilView(tex.Get(), &dsvDesc, dsv.GetAddressOf());

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvRODesc = dsvDesc;
    dsvRODesc.Flags = D3D11_DSV_READ_ONLY_DEPTH;

    ComPtr<ID3D11DepthStencilView> dsvRO;
    dev->CreateDepthStencilView(tex.Get(), &dsvRODesc, dsvRO.GetAddressOf());

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format                    = srvFmt;
    srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels       = 1;

    ComPtr<ID3D11ShaderResourceView> srv;
    dev->CreateShaderResourceView(tex.Get(), &srvDesc, srv.GetAddressOf());

    depth.tex      = tex;
    depth.dsv      = dsv;
    depth.dsvRO    = dsvRO;
    depth.srv      = srv;
    depth.texFmt   = texFmt;
}


void RenderTargetSystem::BindLightAccum()
{
    ID3D11RenderTargetView* rtv = lightAccum.rtv.Get();
    ctx->OMSetRenderTargets(1, &rtv, nullptr);
}

void RenderTargetSystem::BindBackBuffer()
{
    ID3D11RenderTargetView* rtv = device->GetBackBufferRTV();
    ctx->OMSetRenderTargets(1, &rtv, nullptr);
}
// === Pass ===
void RenderTargetSystem::BeginGeometryPass(bool clear)
{
    UnbindAllStageSRVs(0, 16);

    ID3D11RenderTargetView* rtvs[gbufferCount] =
    {
        gbuffer[(size_t)GBufferSlot::RT0].rtv.Get(),
        gbuffer[(size_t)GBufferSlot::RT1].rtv.Get(),
        gbuffer[(size_t)GBufferSlot::RT2].rtv.Get(),
    };
    ctx->OMSetRenderTargets((_uint)gbufferCount, rtvs, depth.dsv.Get());

    if (clear)
    { 
        ClearGBuffer();
        ClearDepth();
    }
}

void RenderTargetSystem::EndGeometryPass()
{
    ctx->OMSetRenderTargets(0, nullptr, nullptr);
}

void RenderTargetSystem::BeginLightingFullScreen(bool clear)
{
    UnbindAllStageSRVs(0, 16);
    ID3D11RenderTargetView* rtv = lightAccum.rtv.Get();
    ctx->OMSetRenderTargets(1, &rtv, nullptr);

    auto& vp = GAME.GetViewport();
    D3D11_VIEWPORT vpDesc{};
    vpDesc.TopLeftX = 0;
    vpDesc.TopLeftY = 0;
    vpDesc.Width    = (float)vp.Width;
    vpDesc.Height   = (float)vp.Height;
    vpDesc.MinDepth = 0; 
    vpDesc.MaxDepth = 1;
    ctx->RSSetViewports(1, &vpDesc);

    if (clear) 
        ClearLightAccum();
}

void RenderTargetSystem::BeginLightingVolumesRO()
{
    UnbindAllStageSRVs(0, 16);
    ID3D11RenderTargetView* rtv = lightAccum.rtv.Get();
    ID3D11DepthStencilView* dsv = depth.dsvRO.Get();
    ctx->OMSetRenderTargets(1, &rtv, dsv);
}

void RenderTargetSystem::BeginCompositeToBackBuffer(bool clear, bool useDepthRO)
{
    UnbindAllStageSRVs(0, 16);
    ID3D11RenderTargetView* rtv = device->GetBackBufferRTV();
    ID3D11DepthStencilView* dsv = useDepthRO ? depth.dsvRO.Get() : nullptr;

    ctx->OMSetRenderTargets(1, &rtv, dsv);
    if (clear)
    {
        const float zero[4] = { 0, 0, 0, 0 };
        ctx->ClearRenderTargetView(rtv, zero);
    }
}

// === G-Buffer SRV pack ===
void RenderTargetSystem::BindGBufferSRVs(_uint psStartSlot)
{
    ID3D11ShaderResourceView* srvs[4] =
    {
        gbuffer[(size_t)GBufferSlot::RT0].srv.Get(),
        gbuffer[(size_t)GBufferSlot::RT1].srv.Get(),
        gbuffer[(size_t)GBufferSlot::RT2].srv.Get(),
    };
    assert(srvs[0] && srvs[1] && srvs[2]);
    ctx->PSSetShaderResources(psStartSlot, 3, srvs);
}

void RenderTargetSystem::UnBindPSAllSRVs(_uint fromSlot, _uint count)
{
    _uint n = count > 16 ? 16 : count;
    ID3D11ShaderResourceView* nulls[16] = {};
    ctx->PSSetShaderResources(fromSlot, n, nulls);
}
// === Clear ===
void RenderTargetSystem::ClearGBuffer(const _float4& rt0, const _float4& rt1, const _float4& rt2)
{
    const float color0[4] = { rt0.x, rt0.y, rt0.z, rt0.w };
    const float color1[4] = { rt1.x, rt1.y, rt1.z, rt1.w };
    const float color2[4] = { rt2.x, rt2.y, rt2.z, rt2.w };
    ctx->ClearRenderTargetView(gbuffer[(size_t)GBufferSlot::RT0].rtv.Get(), color0);
    ctx->ClearRenderTargetView(gbuffer[(size_t)GBufferSlot::RT1].rtv.Get(), color1);
    ctx->ClearRenderTargetView(gbuffer[(size_t)GBufferSlot::RT2].rtv.Get(), color2);
}

void RenderTargetSystem::ClearLightAccum(const _float3& rgb)
{
    const float color[4] = { rgb.x, rgb.y, rgb.z, 0.f };
    ctx->ClearRenderTargetView(lightAccum.rtv.Get(), color);
}

void RenderTargetSystem::ClearDepth()
{
    ctx->ClearDepthStencilView(depth.dsv.Get(), D3D11_CLEAR_DEPTH , 1.f, 0);
}

void RenderTargetSystem::UnbindAllStageSRVs(_uint fromSlot, _uint count)
{
    _uint num = count > 16 ? 16 : count;
    ID3D11ShaderResourceView* nulls[16] = {};
    ctx->VSSetShaderResources(fromSlot, num, nulls);
    ctx->PSSetShaderResources(fromSlot, num, nulls);
    ctx->CSSetShaderResources(fromSlot, num, nulls);
}

DXGI_FORMAT RenderTargetSystem::PickFormatBySlot(const GBufferSpec& spec, GBufferSlot slot)
{
    switch (slot)
    {
    case GBufferSlot::RT0: return spec.rt0;
    case GBufferSlot::RT1: return spec.rt1;
    case GBufferSlot::RT2: return spec.rt2;
    default:
        assert(false); return DXGI_FORMAT_UNKNOWN;
    }
}