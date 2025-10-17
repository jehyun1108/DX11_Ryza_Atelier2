#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL Renderer
{
public:
	static unique_ptr<Renderer> Create();

	HRESULT Init();
	HRESULT Draw(const RenderScene& scene);

	// State
	void BindSamplers(SHADER stage, TEXSLOT slot, SAMPLER type = SAMPLER::LINEAR);
	void SetRasterizerState(RASTERIZER type);
	void SetDepthState(DEPTHSTATE type, _uint stencilRef = 0);
	void SetBlendState(BLENDSTATE type);

private:
	HRESULT UpdateCBuffers(const RenderScene& scene);
	HRESULT DrawOpaque(const vector<DrawItem>& items);
	HRESULT DrawTransparent(const vector<DrawItem>& items);

	// debug / grid
	void DrawAABBLines(const BoundingBox& worldAABB, const _float4& color);
	void DrawGrid();
	void BindGridState();

	HRESULT DrawLineList(const vector<VertexColor>& vertexColor);
	void    DrawOBBLines(const BoundingOrientedBox& worldOBB, const _float4& color);
	void    DrawSphere(const _float3& center, float radius, const _float4& color, int segments = 48);
	void    DrawColliders(const vector<ColliderProxy>& list);

private:
	GameInstance& game       = GameInstance::GetInstance();
	SystemRegistry& registry = game.GetRegistry();
	
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

	// VertexColor
	shared_ptr<Shader> gridShader;

	// Debug
	ComPtr<ID3D11Buffer> aabbVB{};
	_uint                aabbCapBytes = 0;
};

NS_END