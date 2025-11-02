#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL Renderer
{
public:
	static unique_ptr<Renderer> Create();

	HRESULT Init();
	void Draw(const RenderScene& scene);

	// State
	void BindSamplers(SHADER stage, TEXSLOT slot, SAMPLER type = SAMPLER::LINEAR);
	void SetRasterizerState(RASTERIZER type);
	void SetDepthState(DEPTHSTATE type, _uint stencilRef = 0);
	void SetBlendState(BLENDSTATE type);

private:
	void DrawOpaque(const vector<DrawItem>& items);
	void DrawTransparent(const vector<DrawItem>& items);
	void DrawSkyBox(const RenderScene& scene);

	void UpdateCBuffers(const RenderScene& scene);
	void UpdateBoneCB(const _float4x4* sourceMatrices, _uint sourceCount);

	// debug / grid
	void DrawAABBLines(const BoundingBox& worldAABB, const _float4& color);
	void DrawGrid();
	void BindGridState();

	HRESULT DrawLineList(const vector<VertexColor>& vertexColor);
	void    DrawOBBLines(const BoundingOrientedBox& worldOBB, const _float4& color);
	void    DrawSphere(const _float3& center, float radius, const _float4& color, int segments = 48);
	void    DrawColliders(const vector<ColliderProxy>& list);
	void    DrawUI(const vector<UIDrawItem>& items);

	// Skybox
	void  ApplySkyCull(SkyCull cullMode);
	void  ApplySkyBlend(bool transparent, bool premultiplied);
	SkyDrawLists BuildSkyDrawLists(const vector<SkySubmesh>& submeshes);

private:
	GameInstance&   game     = GameInstance::GetInstance();
	SystemRegistry& registry = game.GetRegistry();
	AssetSystem&    assets   = game.GetAssetSystem();
	
	ID3D11Device*        device{};
	ID3D11DeviceContext* context{};

	// States
	ComPtr<ID3D11RasterizerState>   rasterizerStates[ENUM(RASTERIZER::END)];
	ComPtr<ID3D11DepthStencilState> depthStencilStates[ENUM(DEPTHSTATE::END)];
	ComPtr<ID3D11BlendState>        blendStates[ENUM(BLENDSTATE::END)];
	ComPtr<ID3D11SamplerState>      samplerStates[ENUM(SAMPLER::END)];

	// CBuffer
	shared_ptr<CBuffer> cameraCBuffer{};
	shared_ptr<CBuffer> lightCBuffer{};
	shared_ptr<CBuffer> objCBuffer{};
	shared_ptr<CBuffer> boneCBuffer{};
	shared_ptr<CBuffer> skyCBuffer{};
	shared_ptr<CBuffer> tsCBuffer{};
	shared_ptr<CBuffer> uiCBuffer{};

	vector<_float4x4>   identityBones;

	// VertexColor
	shared_ptr<Shader>   gridShader;
	shared_ptr<Shader>   skyShader;
	shared_ptr<Shader>   uiShader;

	unique_ptr<UIMesh> uiMesh;

	// Debug
	ComPtr<ID3D11Buffer> aabbVB{};
	_uint                aabbCapBytes = 0;
};

NS_END