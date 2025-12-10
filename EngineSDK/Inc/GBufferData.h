#pragma once

NS_BEGIN(Engine)

enum class GBufferSlot { RT0, RT1, RT2, END };
static constexpr size_t gbufferCount = ENUM(GBufferSlot::END);

struct GBufferSpec
{
    DXGI_FORMAT rt0        = DXGI_FORMAT_R8G8B8A8_UNORM;       // Albedo.rgb + AO
    DXGI_FORMAT rt1        = DXGI_FORMAT_R16G16B16A16_FLOAT;   // Normal.xyz + Shininess
    DXGI_FORMAT rt2        = DXGI_FORMAT_R16G16B16A16_FLOAT;   // SpecularColor.rgb + Emissive
    DXGI_FORMAT lightAccum = DXGI_FORMAT_R16G16B16A16_FLOAT;   // HDR accumulation
};
struct RTHandle
{
    ComPtr<ID3D11Texture2D>          tex;
    ComPtr<ID3D11RenderTargetView>   rtv;
    ComPtr<ID3D11ShaderResourceView> srv;
    DXGI_FORMAT                      fmt = DXGI_FORMAT_UNKNOWN;
};
struct DepthHandle
{
    ComPtr<ID3D11Texture2D>          tex;   // R32_TYPELESS
    ComPtr<ID3D11DepthStencilView>   dsv;   // D32_FLOAT
    ComPtr<ID3D11DepthStencilView>   dsvRO; // READ_ONLY_DEPTH
    ComPtr<ID3D11ShaderResourceView> srv;   // R32_FLOAT
    DXGI_FORMAT                      texFmt = DXGI_FORMAT_R32_TYPELESS;
};
NS_END