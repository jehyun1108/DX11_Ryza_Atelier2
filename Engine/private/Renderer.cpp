#include "Enginepch.h"

unique_ptr<Renderer> Renderer::Create()
{
	auto instance = make_unique<Renderer>();
	if (FAILED(instance->Init()))
		return nullptr;
	return instance;
}

HRESULT Renderer::Init()
{
	device = game.GetDevice();
	context = game.GetContext();

	cameraCBuffer = CBuffer::Create(sizeof(CameraProxy));
	lightCBuffer  = CBuffer::Create(sizeof(LightProxy));
	objCBuffer    = CBuffer::Create(sizeof(ObjData));
	boneCBuffer   = CBuffer::Create(sizeof(_float4x4) * MAX_BONES);

	auto& assets = game.GetAssetSystem();
	gridShader = assets.GetShader(L"PC");

	// 1. Rasterizer 
	{
		D3D11_RASTERIZER_DESC desc{};
		desc.FillMode              = D3D11_FILL_SOLID;
		desc.FrontCounterClockwise = false; // 오른손 좌표계 기준

		// CullBack (기본)
		desc.CullMode = D3D11_CULL_BACK;
		HR(device->CreateRasterizerState(&desc, &rasterizerStates[ENUM(RASTERIZER::CULLBACK)]));

		// CullNone 
		desc.CullMode = D3D11_CULL_NONE;
		HR(device->CreateRasterizerState(&desc, &rasterizerStates[ENUM(RASTERIZER::CULLNONE)]));

		// WireFrame
		desc.FillMode = D3D11_FILL_WIREFRAME;
		desc.CullMode = D3D11_CULL_BACK;
		HR(device->CreateRasterizerState(&desc, &rasterizerStates[ENUM(RASTERIZER::WIREFRAME)]));

		// CullFront
		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_FRONT;
		HR(device->CreateRasterizerState(&desc, &rasterizerStates[ENUM(RASTERIZER::CULLFRONT)]));
	}

	// 2. Blend
	{
		D3D11_BLEND_DESC desc{};
		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		// Opaque (기본)
		desc.RenderTarget[0].BlendEnable = false;
		HR(device->CreateBlendState(&desc, &blendStates[ENUM(BLENDSTATE::Opaque)]));

		// AlphaBlend (반투명)
		desc.RenderTarget[0].BlendEnable    = true;
		desc.RenderTarget[0].SrcBlend       = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend      = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;
		HR(device->CreateBlendState(&desc, &blendStates[ENUM(BLENDSTATE::ALPHABLEND)]));

		// Additive (가산 혼합)
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		HR(device->CreateBlendState(&desc, &blendStates[ENUM(BLENDSTATE::ADDITIVE)]));
	}

	// 3. Sampler 
	{
		D3D11_SAMPLER_DESC desc{};
		desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		desc.MinLOD         = 0;
		desc.MaxLOD         = D3D11_FLOAT32_MAX;

		// Point Sampler (점 샘플링)
		desc.Filter   = D3D11_FILTER_MIN_MAG_MIP_POINT;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		HR(device->CreateSamplerState(&desc, &samplerStates[ENUM(SAMPLER::POINT)]));

		// Linear Sampler (선형 필터링)
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		HR(device->CreateSamplerState(&desc, &samplerStates[ENUM(SAMPLER::LINEAR)]));

		// Anisotropic Sampler (비등방성 필터링)
		desc.Filter        = D3D11_FILTER_ANISOTROPIC;
		desc.MaxAnisotropy = 4; // 품질 수준 (1~16)
		HR(device->CreateSamplerState(&desc, &samplerStates[ENUM(SAMPLER::ANISOTROPIC)]));

		// Shadow / Comparison Sampler
		desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL; // 깊이 비교 함수 
		HR(device->CreateSamplerState(&desc, &samplerStates[ENUM(SAMPLER::SHADOW)]));
	}

	// 4. DepthStencil 
	{
		D3D11_DEPTH_STENCIL_DESC desc{};
		desc.StencilEnable = false;

		// Default (깊이 테스트/쓰기 모두 활성화)
		desc.DepthEnable = true;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_LESS;
		HR(device->CreateDepthStencilState(&desc, &depthStencilStates[ENUM(DEPTHSTATE::DEFAULT)]));

		// No Depth Test (깊이 테스트 끄기, UI 등)
		desc.DepthEnable = false;
		HR(device->CreateDepthStencilState(&desc, &depthStencilStates[ENUM(DEPTHSTATE::NO_DEPTHTEST)]));

		// No Depth Write (깊이 테스트는 하지만 기록은 안함, 반투명 객체 등)
		desc.DepthEnable = true;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		HR(device->CreateDepthStencilState(&desc, &depthStencilStates[ENUM(DEPTHSTATE::NO_DEPTHWRITE)]));

		// Ready-only + LESS_EQAUL (overlay) 용
		desc.DepthEnable = true;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		HR(device->CreateDepthStencilState(&desc, &depthStencilStates[ENUM(DEPTHSTATE::NO_DEPTHWRITE_LESSEQUAL)]));
	}

	{
		D3D11_DEPTH_STENCIL_DESC desc{};
		desc.DepthEnable = TRUE;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

		desc.StencilEnable = TRUE;
		desc.StencilReadMask = 0xFF;
		desc.StencilWriteMask = 0xFF;

		// 그려지는 픽셀은 스텐실을 1로 REPLACE
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace = desc.FrontFace;

		HR(device->CreateDepthStencilState(&desc, &depthStencilStates[ENUM(DEPTHSTATE::DEPTHSTENCILWRITE)]));
	}

	// 2) 아웃라인 패스: 스텐실 != 1 인 픽셀만 통과(= 실루엣만)
	{
		D3D11_DEPTH_STENCIL_DESC desc{};
		desc.DepthEnable = TRUE;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

		desc.StencilEnable = TRUE;
		desc.StencilReadMask = 0xFF;
		desc.StencilWriteMask = 0x00;

		desc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace = desc.FrontFace;

		HR(device->CreateDepthStencilState(&desc, &depthStencilStates[ENUM(DEPTHSTATE::DEPTHSTENCIL_NOEQUAL)]));
	}

	return S_OK;
}

HRESULT Renderer::DrawOpaque(const vector<DrawItem>& items)
{
	if (items.empty()) return S_OK;

	SetDepthState(DEPTHSTATE::DEFAULT);
	SetBlendState(BLENDSTATE::Opaque);
	SetRasterizerState(RASTERIZER::CULLBACK);

	const auto& vp = game.GetViewport();
	const HoverState& hover = registry.Get<SelectionSystem>().GetHoverState();

	vector<const RenderProxy*> outlineTargets;
	outlineTargets.reserve(8);

	for (const auto& it : items)
	{
		const RenderProxy& proxy = it.proxy;
		if (!proxy.mesh || !proxy.material || !proxy.mesh->IsRenderable()) continue;

		const bool isHovered = (hover.hovered != 0 && proxy.owner == hover.hovered);

		// Skeletal
		if (proxy.boneMatrices && proxy.boneMatrices->data && proxy.boneMatrices->count > 0)
		{
			const auto& matrices = *proxy.boneMatrices;
			boneCBuffer->SetData(matrices.data, size_t(matrices.count) * sizeof(_float4x4));
			boneCBuffer->Update();
			boneCBuffer->Bind(SHADER::VS, CBUFFERSLOT::BONE);
		}

		ObjData obj{};
		obj.world = proxy.world;
		XMStoreFloat4x4(&obj.invWorld, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&proxy.world))));
		obj.vpSize = { vp.Width, vp.Height };

		objCBuffer->SetData(&obj, sizeof(obj));
		objCBuffer->Update();
		objCBuffer->Bind(SHADER::VS, CBUFFERSLOT::OBJ);

		// Hovered -> Stencil 1 마킹
		if (isHovered)
		{
			SetDepthState(DEPTHSTATE::DEPTHSTENCILWRITE, 1);
			outlineTargets.push_back(&proxy);
		}
		else
			SetDepthState(DEPTHSTATE::DEFAULT);

		proxy.material->Bind(context);
		proxy.mesh->Bind(context);
		proxy.mesh->Draw(context);
		proxy.material->UnBind(context);

		if (isHovered)
			SetDepthState(DEPTHSTATE::DEFAULT);
	}

	// ---------------- Outline Pass ----------------
	if (!outlineTargets.empty())
	{
		SetDepthState(DEPTHSTATE::DEPTHSTENCIL_NOEQUAL, 1);
		SetRasterizerState(RASTERIZER::CULLFRONT);
		SetBlendState(BLENDSTATE::ALPHABLEND);

		for (const RenderProxy* proxy : outlineTargets)
		{
			if (!proxy || !proxy->mesh || !proxy->material || !proxy->mesh->IsRenderable()) continue;

			if (proxy->boneMatrices && proxy->boneMatrices->data && proxy->boneMatrices->count > 0)
			{
				const auto& matrices = *proxy->boneMatrices;
				boneCBuffer->SetData(matrices.data, size_t(matrices.count) * sizeof(_float4x4));
				boneCBuffer->Update();
				//원호 왔다감
				boneCBuffer->Bind(SHADER::VS, CBUFFERSLOT::BONE);
			}

			ObjData outline{};
			outline.world = proxy->world;
			XMStoreFloat4x4(&outline.invWorld, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&proxy->world))));
			outline.vpSize = { vp.Width, vp.Height };
			outline.color = _float4(1, 0, 0, 1); 
			outline.outLinePixels = 2.0f;           

			objCBuffer->SetData(&outline, sizeof(outline));
			objCBuffer->Update();
			objCBuffer->Bind(SHADER::VS | SHADER::PS, CBUFFERSLOT::OBJ);

			proxy->material->Bind(context);
			proxy->mesh->Bind(context);
			proxy->mesh->Draw(context);
			proxy->material->UnBind(context);
		}

		// 상태 복구
		SetRasterizerState(RASTERIZER::CULLBACK);
		SetBlendState(BLENDSTATE::Opaque);
		SetDepthState(DEPTHSTATE::DEFAULT);
	}


	return S_OK;
}

HRESULT Renderer::DrawTransparent(const vector<DrawItem>& items)
{
	if (items.empty()) return S_OK;

	SetDepthState(DEPTHSTATE::NO_DEPTHWRITE);
	SetBlendState(BLENDSTATE::ALPHABLEND);
	SetRasterizerState(RASTERIZER::CULLBACK);

	const auto& vp = game.GetViewport();

	for (const auto& it : items)
	{
		const RenderProxy& proxy = it.proxy;
		if (!proxy.mesh || !proxy.material || !proxy.mesh->IsRenderable()) continue;

		if (proxy.boneMatrices && proxy.boneMatrices->data && proxy.boneMatrices->count > 0)
		{
			const auto& matrices = *proxy.boneMatrices;
			boneCBuffer->SetData(matrices.data, static_cast<size_t>(matrices.count) * sizeof(_float4x4));
			boneCBuffer->Update();
			boneCBuffer->Bind(SHADER::VS, CBUFFERSLOT::BONE);
		}

		ObjData obj{};
		obj.world = proxy.world;
		XMStoreFloat4x4(&obj.invWorld, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&proxy.world))));
		obj.vpSize = { vp.Width, vp.Height };

		objCBuffer->SetData(&obj, sizeof(obj));
		objCBuffer->Update();
		objCBuffer->Bind(SHADER::VS, CBUFFERSLOT::OBJ);

		proxy.material->Bind(context);
		proxy.mesh->Bind(context);
		proxy.mesh->Draw(context);
		proxy.material->UnBind(context);
	}

	SetBlendState(BLENDSTATE::Opaque);
	SetDepthState(DEPTHSTATE::DEFAULT);

	return S_OK;
}


HRESULT Renderer::Draw(const RenderScene& scene)
{
	HR(UpdateCBuffers(scene));
	HR(DrawOpaque(scene.queues.opaque)); // Model 그리는중
	HR(DrawTransparent(scene.queues.transparent));

	DrawGrid(); // 땅바닥에 깔린 Grid

	if (!scene.colliders.empty())  DrawColliders(scene.colliders); // Collider

	return S_OK;
}

HRESULT Renderer::UpdateCBuffers(const RenderScene& scene)
{
	// Camera 
	cameraCBuffer->SetData(&scene.cam, sizeof(scene.cam));
	cameraCBuffer->Update();
	cameraCBuffer->Bind(SHADER::VS | SHADER::PS, CBUFFERSLOT::CAMERA);

	// Light
	if (!scene.lights.empty())
	{
		const LightProxy& light = scene.lights[0];
		lightCBuffer->SetData(&light, sizeof(LightProxy));
		lightCBuffer->Update();
		lightCBuffer->Bind(SHADER::PS, CBUFFERSLOT::LIGHT);
	}
	return S_OK;
}

void Renderer::BindGridState()
{
	gridShader->Bind(context);

	ObjData objData{};
	_mat identity = XMMatrixIdentity();

	XMStoreFloat4x4(&objData.world, identity);
	XMStoreFloat4x4(&objData.invWorld, XMMatrixTranspose(identity));

	objCBuffer->SetData(&objData, sizeof(ObjData));
	objCBuffer->Update();
	objCBuffer->Bind(SHADER::VS, CBUFFERSLOT::OBJ);

	// ----------------------------------------
	SetRasterizerState(RASTERIZER::CULLNONE);
	SetDepthState(DEPTHSTATE::NO_DEPTHWRITE);
}

void Renderer::DrawAABBLines(const BoundingBox& worldAABB, const _float4& color)
{
	_float3 corner[8];
	worldAABB.GetCorners(corner);

	static const int Edge[12][2] = 
	{
		{ 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 }, // bottom
		{ 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 }, // top
		{ 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 }  // verticals
	};

	VertexColor vertex[24];
	for (_uint i = 0; i < 12; ++i)
	{
		vertex[i * 2 + 0] = { corner[Edge[i][0]], color };
		vertex[i * 2 + 1] = { corner[Edge[i][1]], color };
	}
	const _uint bytes = sizeof(vertex);

	// VB
	GpuUtil::EnsureDynamicBuffer(device,  aabbVB.GetAddressOf(), bytes, aabbCapBytes, D3D11_BIND_VERTEX_BUFFER);
	GpuUtil::UploadDynamicBuffer(context, aabbVB.Get(), vertex, bytes);

	// Pipeline
	const _uint stride   = sizeof(VertexColor);
	const _uint offset   = 0;
	ID3D11Buffer* buffer = aabbVB.Get();
	context->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
	context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	SetDepthState(DEPTHSTATE::NO_DEPTHWRITE_LESSEQUAL);
	SetBlendState(BLENDSTATE::ALPHABLEND);

	gridShader->Bind(context);
	context->Draw(24, 0);

	SetDepthState(DEPTHSTATE::DEFAULT);
	SetBlendState(BLENDSTATE::Opaque);
	SetRasterizerState(RASTERIZER::CULLBACK);
}

void Renderer::BindSamplers(SHADER stage, TEXSLOT slot, SAMPLER type)
{
	ID3D11SamplerState* sampler = samplerStates[ENUM(type)].Get();
	assert(sampler && "Attempting to bind a null sampler!");

	UINT slotIdx = ENUM(slot);

	if (stage & SHADER::VS) context->VSSetSamplers(slotIdx, 1, &sampler);
	if (stage & SHADER::PS) context->PSSetSamplers(slotIdx, 1, &sampler);
}



HRESULT Renderer::DrawLineList(const vector<VertexColor>& vertexColor)
{
	if (vertexColor.empty()) return S_OK;
	const UINT bytes = UINT(vertexColor.size() * sizeof(VertexColor));

	GpuUtil::EnsureDynamicBuffer(device, aabbVB.GetAddressOf(), bytes, aabbCapBytes, D3D11_BIND_VERTEX_BUFFER);
	GpuUtil::UploadDynamicBuffer(context, aabbVB.Get(), vertexColor.data(), bytes);

	const UINT stride = sizeof(VertexColor);
	const UINT offset = 0;
	ID3D11Buffer* buf = aabbVB.Get();
	context->IASetVertexBuffers(0, 1, &buf, &stride, &offset);
	context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	gridShader->Bind(context);
	context->Draw((UINT)vertexColor.size(), 0);
	return S_OK;
}

void Renderer::DrawOBBLines(const BoundingOrientedBox& worldOBB, const _float4& color)
{
	_float3 corner[8]; 
	worldOBB.GetCorners(corner);
	static const int Edge[12][2] = 
	{
		{ 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 },
		{ 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 },
		{ 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 },
	};
	vector<VertexColor> vertexColor;
	vertexColor.reserve(24);
	for (int i = 0; i < 12; ++i)
	{
		vertexColor.push_back({ corner[Edge[i][0]], color });
		vertexColor.push_back({ corner[Edge[i][1]], color });
	}
	DrawLineList(vertexColor);
}

void Renderer::DrawSphere(const _float3& center, float radius, const _float4& color, int segments)
{
	if (radius <= 0.f) return;
	auto ring = [&](int plane)
		{
			vector<VertexColor> vertexColor;
			vertexColor.reserve(segments * 2);

			for (int i = 0; i < segments; ++i)
			{
				float   a0 = (float(i) / segments) * XM_2PI;
				float   a1 = (float(i + 1) / segments) * XM_2PI;
				_float3 p0 = center;
				_float3 p1 = center;

				switch (plane) 
				{
				case 0: p0.x += radius * cosf(a0); p0.y += radius * sinf(a0);
					    p1.x += radius * cosf(a1); p1.y += radius * sinf(a1); 
					break; 

				case 1: p0.x += radius * cosf(a0); p0.z += radius * sinf(a0);
					    p1.x += radius * cosf(a1); p1.z += radius * sinf(a1); 
					break;

				case 2: p0.y += radius * cosf(a0); p0.z += radius * sinf(a0);
					    p1.y += radius * cosf(a1); p1.z += radius * sinf(a1);
					break;
				}

				vertexColor.push_back({ p0, color }); 
				vertexColor.push_back({ p1, color });
			}
			DrawLineList(vertexColor);
		};
	ring(0); ring(1); ring(2);
}

void Renderer::DrawColliders(const vector<ColliderProxy>& list)
{
	if (list.empty()) return;

	SetDepthState(DEPTHSTATE::NO_DEPTHWRITE_LESSEQUAL);
	SetBlendState(BLENDSTATE::ALPHABLEND);
	SetRasterizerState(RASTERIZER::CULLNONE);

	const _float4 color = _float4(0.f, 1.f, 0.f, 1.f);

	for (const auto& collider : list)
	{
		switch (collider.type)
		{
		case ColliderType::AABB:  
			DrawAABBLines(collider.aabb, color);
			break;

		case ColliderType::OBB:  
			DrawOBBLines(collider.obb, color); 
			break;

		case ColliderType::Sphere:
			DrawSphere(collider.sphereCenter, collider.sphereRadius, color);
			break;
		}
	}

	SetDepthState(DEPTHSTATE::DEFAULT);
	SetBlendState(BLENDSTATE::Opaque);
	SetRasterizerState(RASTERIZER::CULLBACK);
}

void Renderer::DrawGrid()
{
	auto& gridSys = registry.Get<GridSystem>();

	BindGridState();

	SetBlendState(BLENDSTATE::Opaque);
	gridSys.RenderAllLines(context);

	SetBlendState(BLENDSTATE::ALPHABLEND);
	gridSys.RenderAllHover(context);

	// Reset
	SetDepthState(DEPTHSTATE::DEFAULT);
	SetBlendState(BLENDSTATE::Opaque);
	SetRasterizerState(RASTERIZER::CULLBACK);
}

void Renderer::SetRasterizerState(RASTERIZER type)
{
	context->RSSetState(rasterizerStates[ENUM(type)].Get());
}

void Renderer::SetDepthState(DEPTHSTATE type, _uint stencilRef)
{
	context->OMSetDepthStencilState(depthStencilStates[ENUM(type)].Get(), stencilRef);
}

void Renderer::SetBlendState(BLENDSTATE type)
{
	context->OMSetBlendState(blendStates[ENUM(type)].Get(), nullptr, 0xffffffff);
}