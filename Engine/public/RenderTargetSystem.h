#pragma once

#include "GBufferData.h"

NS_BEGIN(Engine)
// Deferred Rendering 에서 GBuffer, LightAccum, Depth 같은 렌더 타깃의 생성,관리,바인딩,클리어를 전담하는 시스템으로 
// 해상도 변화에도 안정적으로 렌더링 자원을 유지하면서 각 패스의 출력 타겟을 자동으로 전환해주는 핵심 인프라 클래스.
class ENGINE_DLL RenderTargetSystem : public ISystem 
{
public:
	explicit RenderTargetSystem(SystemRegistry& registry);

	void Init(_uint width, _uint height, const GBufferSpec& spec);
	void Resize(_uint width, _uint height);
	void Destroy();

	// RTV Binding
	void BindLightAccum();
	void BindBackBuffer();

	// Pass
	void BeginGeometryPass(bool clear = true);
	void EndGeometryPass();
	void BeginLightingFullScreen(bool clear = true);
	void BeginLightingVolumesRO();
	void BeginCompositeToBackBuffer(bool clear = false, bool useDepthRO = true);

	// G-Buffer SRV 묶음 바인딩
	void BindGBufferSRVs(_uint psStartSlot);
	void UnBindPSAllSRVs(_uint fromSlot = 0, _uint count = 16);

	void ClearGBuffer(const _float4&    rt0 = {0,0,0,1},        // AO=1
		              const _float4&    rt1 = {0.5f,0.5f,32,0}, // N=(0,0,1), shininess=32
		              const _float4&    rt2 = {0,0,0,0});
	void ClearLightAccum(const _float3& rgb = {0,0,0});
	void ClearDepth();

	ID3D11ShaderResourceView* GetGBufferSRV(GBufferSlot slot) const { return gbuffer[static_cast<size_t>(slot)].srv.Get(); }
	ID3D11RenderTargetView*   GetGBufferRTV(GBufferSlot slot) const { return gbuffer[static_cast<size_t>(slot)].rtv.Get(); }
	ID3D11ShaderResourceView* GetDepthSRV()                   const { return depth.srv.Get(); }
	ID3D11DepthStencilView*   GetDSV()                        const { return depth.dsv.Get(); }
	ID3D11DepthStencilView*   GetDSVReadOnly()                const { return depth.dsvRO.Get(); }
	ID3D11ShaderResourceView* GetLightAccumSRV()              const { return lightAccum.srv.Get(); }
	ID3D11RenderTargetView*   GetLightAccumRTV()              const { return lightAccum.rtv.Get(); }

	_uint                     GetWidth()                      const { return width; }
	_uint                     GetHeight()                     const { return height; }
	const GBufferSpec&        Spec()                          const { return spec; }

public:
	void InitDressing(_uint _width, _uint _height);
	void BeginDressingPass(bool clear = true);
	void EndDressingPass();

	ID3D11ShaderResourceView* GetDressingSRV() const { return dressingColor.srv.Get(); }

private:
	void                      CreateGBuffer();
	void                      CreateLightAccum();
	void                      CreateDepth();

	void                      UnbindAllStageSRVs(_uint fromSlot, _uint count);
	static DXGI_FORMAT        PickFormatBySlot(const GBufferSpec& spec, GBufferSlot slot);

private:
	SystemRegistry&      registry;
	GameInstance&        game;
	Device*              device{};
	ID3D11Device*        dev{};
	ID3D11DeviceContext* ctx{};

	_uint                width{};
	_uint                height{};
	GBufferSpec          spec{};
				         
	RTHandle             gbuffer[gbufferCount];
	DepthHandle          depth;
	RTHandle             lightAccum;

private:
	RTHandle             minimapColor;
	DepthHandle          minimapDepth;
	_uint                minimapWidth{};
	_uint                minimapHeight{};

private:
	RTHandle    dressingColor;
	DepthHandle dressingDepth;
	_uint       dressingWidth = 0;
	_uint       dressingHeight = 0;

	D3D11_VIEWPORT dressingPrevVP{};
	bool           dressingPrevSaved = false;
};

NS_END