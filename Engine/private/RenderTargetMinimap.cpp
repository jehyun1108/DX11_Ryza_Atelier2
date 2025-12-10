#include "Enginepch.h"
#include "RenderTargetMinimap.h"

RenderTargetMinimap::RenderTargetMinimap(SystemRegistry& registry) : registry(registry), game(GAME)
{
	device = game.GetDevicePtr();
	dev    = game.GetDevice();
	ctx    = game.GetContext();
}

void RenderTargetMinimap::OnBoot()
{
	MinimapRTSpec spec;
	Init(spec);
}

void RenderTargetMinimap::Init(const MinimapRTSpec& spec)
{
	this->spec = spec;
	CreateResources();
}

void RenderTargetMinimap::Resize(_uint width, _uint height)
{
	spec.width = width;
	spec.height = height;
	Destroy();
	CreateResources();
}

void RenderTargetMinimap::Destroy()
{
	ctx->OMSetRenderTargets(0, nullptr, nullptr);

	color.srv.Reset();
	color.rtv.Reset();
	color.tex.Reset();
	color.fmt = DXGI_FORMAT_UNKNOWN;

	depth.srv.Reset();
	depth.dsv.Reset();
	depth.dsvRO.Reset();
	depth.tex.Reset();
	depth.texFmt = spec.depthTexFmt;

	ZeroMemory(&vp, sizeof(vp));
}

void RenderTargetMinimap::CreateResources()
{
	CreateColor();
	CreateDepth();

	vp.TopLeftX = 0.f;
	vp.TopLeftY = 0.f;
	vp.Width = static_cast<float>(spec.width);
	vp.Height = static_cast<float>(spec.height);
	vp.MinDepth = 0.f;
	vp.MaxDepth = 1.f;
}

void RenderTargetMinimap::CreateColor()
{
	auto tex = device->CreateTex2D(spec.width, spec.height,spec.colorFmt, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
	auto rtv = device->CreateRTV(tex.Get());
	auto srv = device->CreateSRV(tex.Get());

	color.tex = tex;
	color.rtv = rtv;
	color.srv = srv;
	color.fmt = spec.colorFmt;
}

void RenderTargetMinimap::CreateDepth()
{
	DXGI_FORMAT texFmt = spec.depthTexFmt;
	DXGI_FORMAT dsvFmt = spec.depthDsvFmt;
	DXGI_FORMAT srvFmt = spec.depthSrvFmt;

	auto tex = device->CreateTex2D(spec.width, spec.height,
		texFmt,
		D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = dsvFmt;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	ComPtr<ID3D11DepthStencilView> dsv;
	dev->CreateDepthStencilView(tex.Get(), &dsvDesc, dsv.GetAddressOf());

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvRODesc = dsvDesc;
	dsvRODesc.Flags = D3D11_DSV_READ_ONLY_DEPTH;
	ComPtr<ID3D11DepthStencilView> dsvRO;
	dev->CreateDepthStencilView(tex.Get(), &dsvRODesc, dsvRO.GetAddressOf());

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = srvFmt;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	ComPtr<ID3D11ShaderResourceView> srv;
	dev->CreateShaderResourceView(tex.Get(), &srvDesc, srv.GetAddressOf());

	depth.tex      = tex;
	depth.dsv      = dsv;
	depth.dsvRO    = dsvRO;
	depth.srv      = srv;
	depth.texFmt   = texFmt;
}

void RenderTargetMinimap::Clear(const _float4& clearColor)
{
	const float c[4] = { clearColor.x, clearColor.y, clearColor.z, clearColor.w };
	ctx->ClearRenderTargetView(color.rtv.Get(), c);
	ctx->ClearDepthStencilView(depth.dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
}

void RenderTargetMinimap::BeginPass(bool clear)
{
	ID3D11RenderTargetView* rtv = color.rtv.Get();
	ctx->OMSetRenderTargets(1, &rtv, depth.dsv.Get());
	ctx->RSSetViewports(1, &vp);

	if (clear)
		Clear(_float4{ 0.4f, 0.35f, 0.3f, 1.f });
}

void RenderTargetMinimap::EndPass()
{
	ctx->OMSetRenderTargets(0, nullptr, nullptr);
}