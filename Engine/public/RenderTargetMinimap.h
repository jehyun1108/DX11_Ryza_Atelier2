#pragma once

#include "RenderTargetMinimapData.h"

NS_BEGIN(Engine)

class ENGINE_DLL RenderTargetMinimap : public ISystem
{
public:
	explicit RenderTargetMinimap(SystemRegistry& registry);
	void     OnBoot() override;

	void Init(const MinimapRTSpec& spec);
	void Resize(_uint width, _uint height);
	void Destroy();

	void BeginPass(bool clear = true);
	void EndPass();

	ID3D11ShaderResourceView* GetColorSRV() const { return color.srv.Get(); }
	ID3D11ShaderResourceView* GetDepthSRV() const { return depth.srv.Get(); }
	ID3D11RenderTargetView*   GetColorRTV() const { return color.rtv.Get(); }
	ID3D11DepthStencilView*   GetDSV()      const { return depth.dsv.Get(); }
	const D3D11_VIEWPORT&     GetViewport() const { return vp; }

private:
	void CreateResources();
	void CreateColor();
	void CreateDepth();
	void Clear(const _float4& clearColor = _float4{ 0.f, 0.f, 0.f, 0.f });

private:
	SystemRegistry&      registry;
	GameInstance&        game;
	Device*              device{};
	ID3D11Device*        dev{};
	ID3D11DeviceContext* ctx{};

	MinimapRTSpec        spec{};
	RTHandle             color;
	DepthHandle          depth;
	D3D11_VIEWPORT       vp{};
};

NS_END