#pragma once

#include "RingCBAllocator.h"
#include "StructuredBoneAllocator.h"
#include "RendererData.h"

NS_BEGIN(Engine)

class ENGINE_DLL Renderer : public ISystem
{
public:
	explicit Renderer(SystemRegistry& registry) : registry(registry), game(GAME) {}
	void     OnBoot() override;
	HRESULT  Init();
	void     Draw(const RenderScene& scene);
	// State
	void BindSamplers(SHADER stage, TEXSLOT slot, SAMPLER type = SAMPLER::LINEAR);
	void SetRasterizerState(RASTERIZER type)                  { context->RSSetState(rasterizerStates[ENUM(type)].Get());                           }
	void SetDepthState(DEPTHSTATE type, _uint stencilRef = 0) { context->OMSetDepthStencilState(depthStencilStates[ENUM(type)].Get(), stencilRef); }
	void SetBlendState(BLENDSTATE type)                       { context->OMSetBlendState(blendStates[ENUM(type)].Get(), nullptr, 0xffffffff);      }

	shared_ptr<Shader>              GetRendererShader(RendererShader id)  { return shaders[(size_t)id]; }
	ComPtr<ID3D11DepthStencilState> GetDepthState(DEPTHSTATE state) const { return depthStencilStates[ENUM(state)]; }

private:
	// Deferred Pipeline
	void DrawOpaque_GBuffer(vector<DrawItem>& items);
	void DrawLighting_FullScreenDirectional(const RenderScene& scene);
	void DrawLighting_Volumes(const RenderScene& scene) {}
	void DrawComposite_ToneMap();
	void DrawMinimapScene(const RenderScene& scene);
	void DrawParticles(const ParticleSnapshot& snapshot);
	void DrawTrails(const RenderScene& scene);
	void DrawDressingCharacter();

	// Forward after Composite
	void DrawTransparent(const vector<DrawItem>& items);
	void DrawSkyBox(const RenderScene& scene);
	void DrawUI(const vector<UIDrawItem>& items);
	void DrawGrid();
	void DrawDebugGBuffers();
	void DrawColliders(const RenderScene& scene);
	// Helper
	void         PrepareObjCBs(vector<DrawItem>& items);
	void         PrepareSkinnedBones(vector<DrawItem>& items);
	void         UpdateCBuffers(const RenderScene& scene);

	void         BindGridState();
	void         ApplySkyCull(SkyCull cullMode);
	void         ApplySkyBlend(bool transparent, bool premultiplied);
	SkyDrawLists BuildSkyDrawLists(const vector<SkySubmesh>& submeshes);

	HRESULT      DrawLineList(const vector<VertexColor>& vertexColor);
	HRESULT      DrawTriList(const vector<VertexColor>& vertexColor);
	void         DrawAABBLines(const BoundingBox& worldAABB, const _float4& color);
	void         DrawFullscreenTriangle();
	void         DrawNavMesh();

private:
	ID3D11Device*        device{};
	ID3D11DeviceContext* context{};
	// States
	ComPtr<ID3D11RasterizerState>   rasterizerStates[ENUM(RASTERIZER::END)];
	ComPtr<ID3D11DepthStencilState> depthStencilStates[ENUM(DEPTHSTATE::END)];
	ComPtr<ID3D11BlendState>        blendStates[ENUM(BLENDSTATE::END)];
	ComPtr<ID3D11SamplerState>      samplerStates[ENUM(SAMPLER::END)];
	// CBuffer			        
	CBufferBank                 cb;
	RingCBAllocator             cbRing;
	StructuredBoneAllocator     boneSB;

	array<shared_ptr<Shader>, rsCount> shaders;
	RenderTargetMinimap*      minimap{};
	unique_ptr<UIMesh>        uiMesh;
	unique_ptr<ParticleMesh>  particleMesh;
	unique_ptr<TrailMesh>     trailMesh;
	// Debug
	ComPtr<ID3D11Buffer>      aabbVB{};
	_uint                     aabbCapBytes = 0;

private:
	GameInstance&           game;
	SystemRegistry&         registry;
	AssetSystem*            assets{};
	GridSystem*             gridSys{};
	SelectionSystem*        selectSys{};
	RenderTargetSystem*     rtSys{};
	NavMeshSystem*          nav{};
	UIRegistry*             uiRegistry{};
	FieldMinimapPresenter*  fieldMini{};
	ScreenDistortionSystem* distortSys{};
	DressingRoomPresenter*  dressing{};
};

NS_END