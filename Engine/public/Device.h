#pragma once

#include "DeviceOptions.h"

NS_BEGIN(Engine)

class ENGINE_DLL Device final
{
public:
	static unique_ptr<Device> Create(WINMODE isWindowed, const DeviceOptions& options = {});

	HRESULT Init(WINMODE isWindowed, const DeviceOptions& options = {});
	HRESULT ClearBackBufferView(const _float4 color);
	HRESULT ClearDSV();
	HRESULT Present();
	
	ID3D11Device*        GetDevice()    const { return device.Get(); }
	ID3D11DeviceContext* GetContext()   const { return context.Get(); }
	IDXGISwapChain1*     GetSwapChain() const { return swapChain.Get(); }
	
	ID3D11RenderTargetView*   GetBackBufferRTV() const { return backBufferRTV.Get(); }
	ID3D11DepthStencilView*   GetDSV()           const { return dsv.Get(); }
	ID3D11ShaderResourceView* GetDepthSRV()      const { return depthSRV.Get(); }
	const D3D11_VIEWPORT&     GetViewport()      const { return viewport; }

	void OnResize(_uint newX, _uint newY);
	void ReleaseDevice();

private:
	HRESULT CreateDeviceAndContext();
	HRESULT ReadySwapChain(WINMODE isWindowed, _uint winX, _uint winY);
	HRESULT ReadyBackBufferRTV();
	HRESULT ReadyDepthTargets(_uint winX, _uint winY);

	bool QueryAllowTearing() const;
	void ApplyFrameLimiter();

private:
	ComPtr<ID3D11Device>              device{};
	ComPtr<ID3D11DeviceContext>       context{};
	ComPtr<IDXGISwapChain1>           swapChain{};
	ComPtr<ID3D11RenderTargetView>    backBufferRTV{};
	ComPtr<ID3D11DepthStencilView>    dsv{};
	ComPtr<ID3D11ShaderResourceView>  depthSRV{};

	D3D11_VIEWPORT viewport{};
	DeviceOptions  opts{};
	bool           allowTearing = false;

	high_resolution_clock::time_point lastPresentTime{};
};

NS_END