#pragma once

#include "DeviceOptions.h"

NS_BEGIN(Engine)

class ENGINE_DLL Device final
{
public:
	function<void(_uint, _uint)> onResized;

	static unique_ptr<Device> Create(WINMODE isWindowed, const DeviceOptions& options = {});
	HRESULT                   Init(WINMODE isWindowed, const DeviceOptions& options = {});
	HRESULT                   ClearBackBufferView(const _float4 color);
	HRESULT                   Present();
	
	ID3D11Device*             GetDevice()        const { return device.Get(); }
	ID3D11DeviceContext*      GetContext()       const { return context.Get(); }
	IDXGISwapChain1*          GetSwapChain()     const { return swapChain.Get(); }
	ID3D11RenderTargetView*   GetBackBufferRTV() const { return backBufferRTV.Get(); }
	const D3D11_VIEWPORT&     GetViewport()      const { return viewport; }

	ComPtr<ID3D11Texture2D>           CreateTex2D(_uint width, _uint height, DXGI_FORMAT fmt, _uint bindFlags, _uint mipLevels = 1);
	ComPtr<ID3D11RenderTargetView>    CreateRTV(ID3D11Texture2D* tex, DXGI_FORMAT fmt = DXGI_FORMAT_UNKNOWN);
	ComPtr<ID3D11ShaderResourceView>  CreateSRV(ID3D11Texture2D* tex, DXGI_FORMAT fmt = DXGI_FORMAT_UNKNOWN);
	ComPtr<ID3D11UnorderedAccessView> CreateUAV(ID3D11Texture2D* tex, DXGI_FORMAT fmt = DXGI_FORMAT_UNKNOWN);

	void OnResize(_uint newX, _uint newY);
	void ReleaseDevice();

private:
	HRESULT CreateDeviceAndContext();
	HRESULT ReadySwapChain(WINMODE isWindowed, _uint winX, _uint winY);
	HRESULT ReadyBackBufferRTV();

	bool    QueryAllowTearing() const;
	void    ApplyFrameLimiter();

private:
	ComPtr<ID3D11Device>              device{};
	ComPtr<ID3D11DeviceContext>       context{};
	ComPtr<IDXGISwapChain1>           swapChain{};
	ComPtr<ID3D11RenderTargetView>    backBufferRTV{};

	D3D11_VIEWPORT                    viewport{};
	DeviceOptions                     opts{};
	bool                              allowTearing = false;
	high_resolution_clock::time_point lastPresentTime{};
};

NS_END