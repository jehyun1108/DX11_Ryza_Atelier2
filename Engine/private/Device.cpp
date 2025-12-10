#include "Enginepch.h"

unique_ptr<Device> Device::Create(WINMODE isWindowed, const DeviceOptions& options)
{
	auto instance = make_unique<Device>();
	if (FAILED(instance->Init(isWindowed, options)))
		return {};
	return instance;
}

HRESULT Device::Init(WINMODE isWindowed, const DeviceOptions& options)
{
	opts = options;
	HR(CreateDeviceAndContext());

	RECT rc{};
	GetClientRect(g_hWnd, &rc);
	const _uint width = rc.right - rc.left;
	const _uint height = rc.bottom - rc.top;

	allowTearing = QueryAllowTearing() && opts.allowTearing;

	HR(ReadySwapChain(isWindowed, width, height));
	HR(ReadyBackBufferRTV());

	ZeroMemory(&viewport, sizeof(viewport));
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width    = static_cast<float>(width);
	viewport.Height   = static_cast<float>(height);
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;
	context->RSSetViewports(1, &viewport);

	lastPresentTime = chrono::high_resolution_clock::now();

	return S_OK;
}

HRESULT Device::CreateDeviceAndContext()
{
	_uint flags = 0;
#ifdef _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	static const D3D_FEATURE_LEVEL levels[] = 
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};
	D3D_FEATURE_LEVEL outLevel{};

	HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0,flags, levels, _countof(levels), 
		D3D11_SDK_VERSION, device.GetAddressOf(), &outLevel, context.GetAddressOf());

#ifdef _DEBUG
	if (FAILED(hr))
	{
		flags &= ~D3D11_CREATE_DEVICE_DEBUG;
		hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0,flags, levels, _countof(levels),
			D3D11_SDK_VERSION, device.GetAddressOf(), &outLevel, context.GetAddressOf());
	}
#endif
	return hr;
}

bool Device::QueryAllowTearing() const
{
	// DXGI 1.5 이상 : IDXGIFactory5::CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING)
	ComPtr<IDXGIDevice> dxgiDevice;
	if (FAILED(device.As(&dxgiDevice))) return false;

	ComPtr<IDXGIAdapter> adapter;
	if (FAILED(dxgiDevice->GetAdapter(&adapter))) return false;

	ComPtr<IDXGIFactory5> factory5;
	if (FAILED(adapter->GetParent(IID_PPV_ARGS(&factory5))))
		return false;

	BOOL allow = FALSE;
	if (FAILED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,
		&allow, sizeof(allow))))
		return false;

	return !!allow;
}

HRESULT Device::ReadySwapChain(WINMODE isWindowed, _uint winX, _uint winY)
{
	ComPtr<IDXGIDevice>   dxgiDevice; 
	HR(device.As(&dxgiDevice));

	ComPtr<IDXGIAdapter>  adapter;    
	HR(dxgiDevice->GetAdapter(&adapter));

	ComPtr<IDXGIFactory2> factory2;    
	HR(adapter->GetParent(IID_PPV_ARGS(&factory2)));

	DXGI_SWAP_CHAIN_DESC1 desc1{};
	desc1.Width       = winX;
	desc1.Height      = winY;
	desc1.Format      = opts.backbufferFormat;   // UNORM (출력은 RTV sRGB로)
	desc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc1.BufferCount = max<_uint>(2, opts.backbufferCount);
	desc1.SampleDesc  = { 1, 0 };
	desc1.Scaling     = DXGI_SCALING_STRETCH;     // 일반 창 크기 변경 시 보편적
	desc1.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD; 
	desc1.AlphaMode   = DXGI_ALPHA_MODE_IGNORE;
	desc1.Flags = ((int)isWindowed && allowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	ComPtr<IDXGISwapChain1> swapChain1;
	HR(factory2->CreateSwapChainForHwnd(device.Get(), g_hWnd, &desc1, nullptr, nullptr, &swapChain1));
	HR(factory2->MakeWindowAssociation(g_hWnd, 0));

	swapChain = swapChain1;
	return S_OK;
}

HRESULT Device::ReadyBackBufferRTV()
{
	if (!device || !swapChain) return E_FAIL;

	ComPtr<ID3D11Texture2D> backBuffer;
	HR(swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));

	// sRGB 출력 옵션에 따른 RTV 포맷 설정
	DXGI_FORMAT rtvFormat = opts.useSRGBbackbuffer ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format             = rtvFormat;
	rtvDesc.Texture2D.MipSlice = 0;

	HR(device->CreateRenderTargetView(backBuffer.Get(), &rtvDesc, backBufferRTV.GetAddressOf()))

	return S_OK;
}

HRESULT Device::ClearBackBufferView(const _float4 color)
{
	if (!context || !backBufferRTV) return E_FAIL;
	
	const float _color[4] = { color.x, color.y, color.z, color.w };
	context->ClearRenderTargetView(backBufferRTV.Get(), _color);

	return S_OK;
}

void Device::ApplyFrameLimiter()
{
	if (opts.vsyncMode != VSyncMode::Off || opts.frameLimiterHz == 0) return;

	const auto   now       = high_resolution_clock::now();
	const double targetMs  = 1000.0 / static_cast<double>(opts.frameLimiterHz);
	const double elapsedMs = duration<double, milli>(now - lastPresentTime).count();

	if (elapsedMs < targetMs)
	{
		const auto sleepMs = targetMs - elapsedMs;
		if (sleepMs > 0.5)
			this_thread::sleep_for(milliseconds((int)floor(sleepMs)));
	}
	lastPresentTime = high_resolution_clock::now();
}

HRESULT Device::Present()
{
	if (!swapChain) return E_FAIL;

	ApplyFrameLimiter();

	_uint sync  = 0;
	_uint flags = 0;

	switch (opts.vsyncMode)
	{
	case VSyncMode::On:
		sync = 1; flags = 0;
		break;

	case VSyncMode::Off:
		sync = 0; flags = allowTearing ? DXGI_PRESENT_ALLOW_TEARING : 0;
		break;

	case VSyncMode::Adaptive:
		sync = 1; flags = 0;
		break;

	default: break;
	}
	
	return swapChain->Present(sync, flags);
}

ComPtr<ID3D11Texture2D> Device::CreateTex2D(_uint width, _uint height, DXGI_FORMAT fmt, _uint bindFlags, _uint mipLevels)
{
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width      = width;
	desc.Height     = height;
	desc.MipLevels  = mipLevels;
	desc.ArraySize  = 1;
	desc.Format     = fmt;
	desc.SampleDesc = { 1, 0 };
	desc.Usage      = D3D11_USAGE_DEFAULT;
	desc.BindFlags  = bindFlags;
	ComPtr<ID3D11Texture2D> tex;
	device->CreateTexture2D(&desc, nullptr, tex.GetAddressOf());
	return tex;
}

ComPtr<ID3D11RenderTargetView> Device::CreateRTV(ID3D11Texture2D* tex, DXGI_FORMAT fmt)
{
	D3D11_TEXTURE2D_DESC texDesc{};
	tex->GetDesc(&texDesc);
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format             = (fmt == DXGI_FORMAT_UNKNOWN) ? texDesc.Format : fmt;
	rtvDesc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	ComPtr<ID3D11RenderTargetView> rtv;
	device->CreateRenderTargetView(tex, &rtvDesc, rtv.GetAddressOf());
	return rtv;
}

ComPtr<ID3D11ShaderResourceView> Device::CreateSRV(ID3D11Texture2D* tex, DXGI_FORMAT fmt)
{
	D3D11_TEXTURE2D_DESC texDesc{};
	tex->GetDesc(&texDesc);
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = (fmt == DXGI_FORMAT_UNKNOWN) ? texDesc.Format : fmt;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	ComPtr<ID3D11ShaderResourceView> srv;
	device->CreateShaderResourceView(tex, &srvDesc, srv.GetAddressOf());
	return srv;
}

ComPtr<ID3D11UnorderedAccessView> Device::CreateUAV(ID3D11Texture2D* tex, DXGI_FORMAT fmt)
{
	D3D11_TEXTURE2D_DESC texDesc{}; 
	tex->GetDesc(&texDesc);
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = (fmt == DXGI_FORMAT_UNKNOWN) ? texDesc.Format : fmt;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	ComPtr<ID3D11UnorderedAccessView> uav;
	device->CreateUnorderedAccessView(tex, &uavDesc, uav.GetAddressOf());
	return uav;
}

void Device::OnResize(_uint newX, _uint newY)
{
	if (!swapChain || !context || !device) return;

	context->OMSetRenderTargets(0, nullptr, nullptr);
	ID3D11ShaderResourceView* nullSRV[1] = {nullptr};
	context->PSSetShaderResources(0, 1, nullSRV);

	backBufferRTV.Reset();

	HR(swapChain->ResizeBuffers(max<_uint>(2, opts.backbufferCount), newX, newY, opts.backbufferFormat, allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0));

	ReadyBackBufferRTV();

	viewport.Width  = static_cast<float>(newX);
	viewport.Height = static_cast<float>(newY);
	context->RSSetViewports(1, &viewport);
}

void Device::ReleaseDevice()
{
	backBufferRTV.Reset();
	swapChain.Reset();
	context.Reset();

#ifdef _DEBUG
	if (device)
	{
		ComPtr<ID3D11Debug> debug;
		if (SUCCEEDED(device.As(&debug)))
		{
		//	OutputDebugStringW(L"--- D3D11 Live Objects ---\r\n");
		//	debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		//	OutputDebugStringW(L"--- D3D11 Live Objects END ---\r\n");
		}
	}
#endif
	device.Reset();
}