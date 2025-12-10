#include "Enginepch.h"

#include "DebugUtility.h"
#include "NavMeshSystem.h"
#include "RenderTargetMinimap.h"
#include "FieldMinimapPresenter.h"
#include "ScreenDistortionSystem.h"
#include "DressingRoomPresenter.h"
#include "Renderer.h"

static inline _uint AlignConst16(_uint c) { return (c + 15u) & ~15u; }
// ============================================================================
void Renderer::OnBoot()
{
	assets     = &registry.Get<AssetSystem>();
	gridSys    = &registry.Get<GridSystem>();
	selectSys  = &registry.Get<SelectionSystem>();
	rtSys      = &registry.Get<RenderTargetSystem>();
	nav        = &registry.Get<NavMeshSystem>();
	minimap    = &registry.Get<RenderTargetMinimap>();
	uiRegistry = &registry.Get<UIRegistry>();
	fieldMini  = &registry.Get<FieldMinimapPresenter>();
	distortSys = &registry.Get<ScreenDistortionSystem>();
	dressing   = &registry.Get<DressingRoomPresenter>();
	Init();
}

HRESULT Renderer::Init()
{
	device  = game.GetDevice();
	context = game.GetContext();

	for (size_t i = 0; i < (size_t)RendererShader::Count; ++i)
		shaders[i] = assets->GetShader(wstring(rsKeys[i]));

	uiMesh = make_unique<UIMesh>();
	uiMesh->Create(device, 1024);

	particleMesh = make_unique<ParticleMesh>();
	particleMesh->Create(device, 8192);

	trailMesh = make_unique<TrailMesh>();
	trailMesh->Create(device, 8192, 12288);

	cbRing.Init(device, context, 16384);
	boneSB.Init(device, context, sizeof(_float4x4), 8192);

	array<_uint, CBufferCount> sizes =
	{
		(_uint)sizeof(CameraProxy),             // Camera
		(_uint)sizeof(LightProxy),              // Light
		(_uint)sizeof(ObjCB),                   // Obj
		(_uint)sizeof(SkyCB),                   // Sky
		(_uint)sizeof(UICB),                    // UI
		(_uint)sizeof(MaterialCB),
		(_uint)sizeof(PostCB),
		(_uint)sizeof(DebugCB),
		(_uint)sizeof(DistortionCB),
		(_uint)sizeof(TrailCB),
	};

	cb.Init(device, context, sizes);

	{
		D3D11_RASTERIZER_DESC desc{};
		desc.FillMode              = D3D11_FILL_SOLID;
		desc.FrontCounterClockwise = false; // 오른손 좌표계 기준

		desc.CullMode = D3D11_CULL_BACK;
		HR(device->CreateRasterizerState(&desc, &rasterizerStates[ENUM(RASTERIZER::CULLBACK)]));

		desc.CullMode = D3D11_CULL_NONE;
		HR(device->CreateRasterizerState(&desc, &rasterizerStates[ENUM(RASTERIZER::CULLNONE)]));

		desc.FillMode = D3D11_FILL_WIREFRAME;
		desc.CullMode = D3D11_CULL_BACK;
		HR(device->CreateRasterizerState(&desc, &rasterizerStates[ENUM(RASTERIZER::WIREFRAME)]));

		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_FRONT;
		HR(device->CreateRasterizerState(&desc, &rasterizerStates[ENUM(RASTERIZER::CULLFRONT)]));

		desc.FillMode        = D3D11_FILL_SOLID;
		desc.CullMode        = D3D11_CULL_NONE;
		desc.ScissorEnable   = TRUE;
		desc.DepthClipEnable = TRUE;
		HR(device->CreateRasterizerState(&desc, &rasterizerStates[ENUM(RASTERIZER::UI_SCISSOR)]));
	}
	{
		D3D11_BLEND_DESC desc{};
		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		desc.RenderTarget[0].BlendEnable = false;
		HR(device->CreateBlendState(&desc, &blendStates[ENUM(BLENDSTATE::Opaque)]));

		desc.RenderTarget[0].BlendEnable    = true;
		desc.RenderTarget[0].SrcBlend       = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend      = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;
		HR(device->CreateBlendState(&desc, &blendStates[ENUM(BLENDSTATE::ALPHABLEND)]));

		desc.RenderTarget[0].SrcBlend       = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlend      = D3D11_BLEND_ONE;
		HR(device->CreateBlendState(&desc, &blendStates[ENUM(BLENDSTATE::ADDITIVE)]));

		desc.RenderTarget[0].SrcBlend       = D3D11_BLEND_ONE;           
		desc.RenderTarget[0].DestBlend      = D3D11_BLEND_INV_SRC_ALPHA;  
		desc.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		HR(device->CreateBlendState(&desc, &blendStates[ENUM(BLENDSTATE::PM_ALPHA)]));
	}
	{
		D3D11_SAMPLER_DESC desc{};
		desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		desc.MinLOD         = 0;
		desc.MaxLOD         = D3D11_FLOAT32_MAX;

		desc.Filter         = D3D11_FILTER_MIN_MAG_MIP_POINT;
		desc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
		HR(device->CreateSamplerState(&desc, &samplerStates[ENUM(SAMPLER::POINT)]));

		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		HR(device->CreateSamplerState(&desc, &samplerStates[ENUM(SAMPLER::LINEAR)]));

		desc.Filter        = D3D11_FILTER_ANISOTROPIC;
		desc.MaxAnisotropy = 4;
		HR(device->CreateSamplerState(&desc, &samplerStates[ENUM(SAMPLER::ANISOTROPIC)]));
	}
	{
		D3D11_DEPTH_STENCIL_DESC desc{};
		desc.StencilEnable  = false;

		desc.DepthEnable    = true;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc      = D3D11_COMPARISON_LESS;
		HR(device->CreateDepthStencilState(&desc, &depthStencilStates[ENUM(DEPTHSTATE::DEFAULT)]));

		desc.DepthEnable    = false;
		HR(device->CreateDepthStencilState(&desc, &depthStencilStates[ENUM(DEPTHSTATE::NO_DEPTHTEST)]));

		desc.DepthEnable    = true;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		HR(device->CreateDepthStencilState(&desc, &depthStencilStates[ENUM(DEPTHSTATE::NO_DEPTHWRITE)]));

		desc.DepthEnable    = true;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		desc.DepthFunc      = D3D11_COMPARISON_LESS_EQUAL;
		HR(device->CreateDepthStencilState(&desc, &depthStencilStates[ENUM(DEPTHSTATE::NO_DEPTHWRITE_LESSEQUAL)]));
	}
	{
		D3D11_DEPTH_STENCIL_DESC desc{};
		desc.DepthEnable    = TRUE;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc      = D3D11_COMPARISON_LESS_EQUAL;

		desc.StencilEnable    = TRUE;
		desc.StencilReadMask  = 0xFF;
		desc.StencilWriteMask = 0xFF;

		desc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
		desc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_REPLACE;
		desc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace                     = desc.FrontFace;

		HR(device->CreateDepthStencilState(&desc, &depthStencilStates[ENUM(DEPTHSTATE::DEPTHSTENCILWRITE)]));
	}
	{
		D3D11_DEPTH_STENCIL_DESC desc{};
		desc.DepthEnable    = TRUE;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		desc.DepthFunc      = D3D11_COMPARISON_LESS_EQUAL;

		desc.StencilEnable    = TRUE;
		desc.StencilReadMask  = 0xFF;
		desc.StencilWriteMask = 0x00;

		desc.FrontFace.StencilFunc        = D3D11_COMPARISON_NOT_EQUAL;
		desc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace                     = desc.FrontFace;

		HR(device->CreateDepthStencilState(&desc, &depthStencilStates[ENUM(DEPTHSTATE::DEPTHSTENCIL_NOEQUAL)]));
	}
	return S_OK;
}

void Renderer::Draw(const RenderScene& scene)
{
	DrawDressingCharacter();
	UpdateCBuffers(scene);
	DrawMinimapScene(scene);

	// 1. Geometry Pass
	rtSys->BeginGeometryPass(true);
	SetDepthState(DEPTHSTATE::DEFAULT);
	DrawOpaque_GBuffer(const_cast<vector<DrawItem>&>(scene.queues.opaque));
	rtSys->EndGeometryPass();

	// 3. Lighting Pass
	SetDepthState(DEPTHSTATE::NO_DEPTHTEST); 
	rtSys->BeginLightingFullScreen(true);
	rtSys->BindGBufferSRVs(0);      
	BindSamplers(SHADER::PS, (TEXSLOT)0, SAMPLER::LINEAR);
	BindSamplers(SHADER::PS, (TEXSLOT)1, SAMPLER::POINT);
	SetBlendState(BLENDSTATE::ADDITIVE);
	DrawLighting_FullScreenDirectional(scene);    
	SetBlendState(BLENDSTATE::Opaque);

	// 3. Composite
	rtSys->BeginCompositeToBackBuffer();
	DrawComposite_ToneMap();

	//DrawNavMesh();
	DrawSkyBox(scene);
	//DrawTransparent(scene.queues.transparent);
	//DrawGrid();
	DrawParticles(scene.particles);
	DrawTrails(scene);
#ifdef USE_IMGUI
	//DrawColliders(scene);
#endif
	DrawUI(scene.ui.drawItems);
	//DrawDebugGBuffers();
}

void Renderer::BindSamplers(SHADER stage, TEXSLOT slot, SAMPLER type)
{
	ID3D11SamplerState* s = samplerStates[ENUM(type)].Get();
	const UINT idx = ENUM(slot);
	if (stage & SHADER::VS) context->VSSetSamplers(idx, 1, &s);
	if (stage & SHADER::PS) context->PSSetSamplers(idx, 1, &s);
	if (stage & SHADER::HS) context->HSSetSamplers(idx, 1, &s);
	if (stage & SHADER::DS) context->DSSetSamplers(idx, 1, &s);
}
// ================== Pipeline stages ================================================================
void Renderer::DrawOpaque_GBuffer(vector<DrawItem>& items)
{
	if (items.empty()) return;

	SetDepthState(DEPTHSTATE::DEFAULT);
	SetBlendState(BLENDSTATE::Opaque);
	SetRasterizerState(RASTERIZER::CULLBACK);

	PrepareSkinnedBones(items);
	PrepareObjCBs(items);

	const auto& vp = game.GetViewport();

	for (const auto& it : items)
	{
		const RenderProxy& proxy = it.proxy;
		if (!proxy.mesh || !proxy.material || !proxy.mesh->IsRenderable()) continue;

		proxy.material->Bind(context);
		proxy.mesh->Bind(context);
		cbRing.BindRange(SHADER::VS, CBUFFERSLOT::OBJ, it.cbFirst, it.cbNum);
		proxy.mesh->Draw(context);
	}
}

void Renderer::DrawMinimapScene(const RenderScene& scene)
{
	if (!scene.minimapEnabled) return;

	D3D11_VIEWPORT mainVP{};
	_uint count = 1;
	context->RSGetViewports(&count, &mainVP);

	minimap->BeginPass(true);
	SetDepthState(DEPTHSTATE::DEFAULT);
	SetBlendState(BLENDSTATE::Opaque);
	SetRasterizerState(RASTERIZER::CULLBACK);

	cb.UpdateBindT(CBufferID::Camera, scene.minimapCam, SHADER::VS | SHADER::PS, CBUFFERSLOT::CAMERA);
	minimap->EndPass();
	context->RSSetViewports(1, &mainVP);
	cb.UpdateBindT(CBufferID::Camera, scene.cam, SHADER::VS | SHADER::PS, CBUFFERSLOT::CAMERA);
}

void Renderer::DrawParticles(const ParticleSnapshot& snapshot)
{
	const auto& items = snapshot.transparent;
	if (items.empty()) return;

	SetDepthState(DEPTHSTATE::NO_DEPTHWRITE_LESSEQUAL);
	SetBlendState(BLENDSTATE::ALPHABLEND);
	SetRasterizerState(RASTERIZER::CULLNONE);

	auto sorted = items;
	sort(sorted.begin(), sorted.end(),
		[](const ParticleDrawItem& a, const ParticleDrawItem& b)
		{
			return a.camDist > b.camDist;
		});

	GetRendererShader(RendererShader::Particle)->Bind(context);

	unordered_map<wstring, vector<const ParticleDrawItem*>> buckets;

	for (const auto& it : sorted)
	{
		const wstring& texKey = it.texKey;      // ← spawnData 대신 texKey 사용
		buckets[texKey].push_back(&it);
	}

	for (auto& pair : buckets)
	{
		const wstring& texKey = pair.first;
		auto& list = pair.second;

		auto tex = assets->GetTexture(texKey);
		assert(tex && "Particle texture not registered");

		ID3D11ShaderResourceView* srv = tex->GetSrv();
		context->PSSetShaderResources(0, 1, &srv);

		particleMesh->Draw(context, list);
	}

	SetBlendState(BLENDSTATE::Opaque);
	SetDepthState(DEPTHSTATE::DEFAULT);
	SetRasterizerState(RASTERIZER::CULLBACK);
}

void Renderer::DrawTrails(const RenderScene& scene)
{
	const auto& src = scene.trails.trails;
	if (src.empty()) return;

	SetDepthState(DEPTHSTATE::NO_DEPTHWRITE_LESSEQUAL);
	SetBlendState(BLENDSTATE::ALPHABLEND);
	SetRasterizerState(RASTERIZER::CULLNONE);

	auto shader = GetRendererShader(RendererShader::Trail);
	shader->Bind(context);

	struct TrailBatch
	{
		wstring           texKey;
		const TrailDesc*  desc = nullptr; 
		TrailSnapshot     snapshot;
	};

	vector<TrailBatch> batches;

	auto getBatch = [&](const wstring& key, const TrailDesc* desc) -> TrailBatch&
		{
			for (auto& b : batches)
				if (b.texKey == key && b.desc == desc)    
					return b;

			TrailBatch nb{};
			nb.texKey = key;
			nb.desc = desc;
			batches.push_back(std::move(nb));
			return batches.back();
		};

	for (const auto& item : src)
	{
		const TrailDesc* desc = item.desc;
		assert(desc);

		const wstring& key = desc->texKey;
		const wstring  useKey = key.empty() ? L"trail_default" : key;

		TrailBatch& batch = getBatch(useKey, desc);
		batch.snapshot.trails.push_back(item);
	}

	for (auto& batch : batches)
	{
		const TrailDesc* desc = batch.desc;
		assert(desc);

		TrailCB tcb{};
		const SpriteSheetInfo& s = desc->sheet;

		if (s.enabled)
		{
			_uint cols = (_uint)max(1, s.cols);
			_uint rows = (_uint)max(1, s.rows);
			_uint startF = (_uint)max(0, s.startFrame);
			_uint endF = (_uint)max(startF, s.endFrame);

			tcb.trailCols = cols;
			tcb.trailRows = rows;
			tcb.trailStartFrame = startF;
			tcb.trailEndFrame = endF;
			tcb.trailFps = s.fps;

			tcb.trailSheetEnabled = 1u;
		}
		else
		{
			tcb.trailCols = 1;
			tcb.trailRows = 1;
			tcb.trailStartFrame = 0;
			tcb.trailEndFrame = 0;
			tcb.trailFps = 0.f;

			tcb.trailSheetEnabled = 0u;
		}

		tcb.trailTime = game.GetGameTime();
		tcb.pad = 0.f;

		cb.UpdateBindT(CBufferID::Trail, tcb, SHADER::PS, CBUFFERSLOT::TRAIL);

		auto tex = assets->GetTexture(batch.texKey);
		ID3D11ShaderResourceView* srv = tex ? tex->GetSrv() : nullptr;
		context->PSSetShaderResources(0, 1, &srv);

		trailMesh->Draw(context, batch.snapshot, scene.cam);
	}

	ID3D11ShaderResourceView* nullSrv[1] = { nullptr };
	context->PSSetShaderResources(0, 1, nullSrv);

	SetBlendState(BLENDSTATE::Opaque);
	SetDepthState(DEPTHSTATE::DEFAULT);
	SetRasterizerState(RASTERIZER::CULLBACK);
}

void Renderer::DrawDressingCharacter()
{
	if (!dressing->IsActive())  return;

	CameraProxy cam{};
	dressing->BuildDressingCamera(cam);
	cb.UpdateBindT(CBufferID::Camera, cam, SHADER::VS | SHADER::PS, CBUFFERSLOT::CAMERA);

	rtSys->BeginDressingPass(true);

	SetDepthState(DEPTHSTATE::DEFAULT);
	SetBlendState(BLENDSTATE::Opaque);
	SetRasterizerState(RASTERIZER::CULLBACK);

	vector<DrawItem> items;
	dressing->BuildDressingDrawItems(items);
	if (items.empty())
	{
		rtSys->EndDressingPass();
		return;
	}

	PrepareSkinnedBones(items);
	PrepareObjCBs(items);

	auto dressingShader = assets->GetShader(L"Dressing");

	for (auto& it : items)
	{
		const RenderProxy& proxy = it.proxy;
		if (!proxy.mesh || !proxy.material || !proxy.mesh->IsRenderable())
			continue;
		proxy.material->Bind(context);
		dressingShader->Bind(context);

		proxy.mesh->Bind(context);
		cbRing.BindRange(SHADER::VS, CBUFFERSLOT::OBJ, it.cbFirst, it.cbNum);
		proxy.mesh->Draw(context);
	}

	rtSys->EndDressingPass();
}

void Renderer::PrepareObjCBs(vector<DrawItem>& items)
{
	if (items.empty()) return;

	const auto& vp = game.GetViewport();
	cbRing.BeginFrameWrite();

	for (auto& it : items)
	{
		const auto& p = it.proxy;
		if (!p.mesh || !p.material || !p.mesh->IsRenderable()) continue;

		ObjCB obj{};
		obj.world     = p.world;
		obj.invWorld  = Utility::Inverse(p.world);
		obj.vpSize    = { (float)vp.Width, (float)vp.Height, 0, 0 }; 
		obj.boneBase  = it.boneBase;
		obj.boneCount = it.boneCount;

		const _uint needConsts  = ObjCBConstants;         
		const _uint allocConsts = AlignConst16(needConsts); 
		const _uint first       = cbRing.Alloc(allocConsts);
		cbRing.Write(first, &obj, ObjCBSizeBytes);

		it.cbFirst = first;
		it.cbNum   = allocConsts; 
	}

	cbRing.EndFrameWrite();
}

void Renderer::PrepareSkinnedBones(vector<DrawItem>& items)
{
	struct BoneRange { _uint base; _uint count; };
	unordered_map<const _float4x4*, _uint> uniqueCounts;
	uniqueCounts.reserve(items.size());

	for (auto& it : items)
	{
		const auto& proxy = it.proxy;
		if (!proxy.isSkinned || !proxy.boneMatrices) continue;
		uniqueCounts[proxy.boneMatrices->data] = proxy.boneMatrices->count;
	}

	_uint total = 0;
	for (auto& pair : uniqueCounts)
		total += pair.second;

	boneSB.EnsureCapacity(total);
	boneSB.BeginFrameWrite();

	unordered_map<const _float4x4*, BoneRange> rangeByPtr;
	rangeByPtr.reserve(uniqueCounts.size());

	for (auto& pair : uniqueCounts)
	{
		const _float4x4* data = pair.first;
		const _uint count = pair.second;
		const _uint base  = boneSB.Alloc(count);
		boneSB.Write(base, data, count);
		rangeByPtr[data] = BoneRange{ base, count };
	}

	boneSB.EndFrameWrite();
	boneSB.BindVS(22);

	for (auto& pair : items)
	{
		const auto& proxy = pair.proxy;
		if (!proxy.isSkinned || !proxy.boneMatrices)
		{
			pair.boneBase  = 0;
			pair.boneCount = 0;
			continue;
		}
		const auto& it = rangeByPtr.find(proxy.boneMatrices->data);
		assert(it != rangeByPtr.end());
		pair.boneBase  = it->second.base;
		pair.boneCount = it->second.count;
	}
}

void Renderer::DrawLighting_FullScreenDirectional(const RenderScene& scene)
{
	if (!scene.lights.empty())
		cb.UpdateBindT(CBufferID::Light, scene.lights[0], SHADER::PS, CBUFFERSLOT::LIGHT);

	GetRendererShader(RendererShader::Deferred_Directional)->Bind(context);
	DrawFullscreenTriangle();
}

void Renderer::DrawComposite_ToneMap()
{
	GetRendererShader(RendererShader::Deferred_Composite)->Bind(context);

	ID3D11ShaderResourceView* srvs[2] =
	{
		rtSys->GetLightAccumSRV(),              
		rtSys->GetGBufferSRV(GBufferSlot::RT0)  
	};
	context->PSSetShaderResources(4, 2, srvs);

	DrawFullscreenTriangle();

	ID3D11ShaderResourceView* nulls[2] = {};
	context->PSSetShaderResources(4, 2, nulls);
}

void Renderer::DrawTransparent(const vector<DrawItem>& items)
{
	if (items.empty()) return;

	SetDepthState(DEPTHSTATE::NO_DEPTHWRITE_LESSEQUAL);
	SetBlendState(BLENDSTATE::ALPHABLEND);
	SetRasterizerState(RASTERIZER::CULLBACK);

	const auto& vp = game.GetViewport();

	for (const auto& it : items)
	{
		const RenderProxy& proxy = it.proxy;
		if (!proxy.mesh || !proxy.material || !proxy.mesh->IsRenderable()) continue;

		proxy.material->Bind(context);
		proxy.mesh->Bind(context);
		proxy.mesh->Draw(context);
	}

	SetBlendState(BLENDSTATE::Opaque);
	SetDepthState(DEPTHSTATE::DEFAULT);
}

void Renderer::DrawSkyBox(const RenderScene& scene)
{
	const auto& proxy = scene.skybox;
	if (!proxy.enabled || proxy.submeshes.empty()) return;

	SetDepthState(DEPTHSTATE::NO_DEPTHWRITE_LESSEQUAL);

	const _float3 camPos = { scene.cam.camPos.x, scene.cam.camPos.y, scene.cam.camPos.z };
	const float theta = proxy.baseYawRad + proxy.phaseRad + (proxy.hasTfYaw ? proxy.tfYawRad : 0.f);

	const _mat mRot = XMMatrixRotationY(theta);
	const _mat mScale = XMMatrixScaling(proxy.uniformScale, proxy.uniformScale, proxy.uniformScale);
	const _mat mTrans = XMMatrixTranslation(camPos.x, camPos.y, camPos.z);

	_float4x4 world{};
	XMStoreFloat4x4(&world, XMMatrixMultiply(XMMatrixMultiply(mRot, mScale), mTrans));

	ObjCB obj{};
	obj.world = world;
	XMStoreFloat4x4(&obj.invWorld, XMMatrixInverse(nullptr, XMLoadFloat4x4(&world)));
	cb.UpdateBindT(CBufferID::Obj, obj, SHADER::VS | SHADER::DS, CBUFFERSLOT::OBJ);

	auto lists = BuildSkyDrawLists(proxy.submeshes);
	auto drawList = [&](const vector<const SkySubmesh*>& list)
		{
			for (auto* sm : list)
			{
				ApplySkyCull(sm->cull);
				ApplySkyBlend(sm->transparent, sm->premultiplied);

				SkyCB sky{};
				sky.theta = theta;
				sky.opacity = sm->opacity;
				sky.isPremultiplied = sm->premultiplied ? 1 : 0;

				cb.UpdateBindT(CBufferID::Sky, sky, SHADER::PS, CBUFFERSLOT::SKY);

				sm->material->Bind(context);
				sm->mesh->Bind(context);
				sm->mesh->Draw(context);
			}
		};
	drawList(lists.opaque);
	drawList(lists.alpha);

	SetBlendState(BLENDSTATE::Opaque);
	SetDepthState(DEPTHSTATE::DEFAULT);
	SetRasterizerState(RASTERIZER::CULLBACK);
}

void Renderer::DrawUI(const vector<UIDrawItem>& items)
{
	if (items.empty()) return;

	SetDepthState(DEPTHSTATE::NO_DEPTHTEST);
	SetBlendState(BLENDSTATE::ALPHABLEND);
	SetRasterizerState(RASTERIZER::UI_SCISSOR);

	const auto& vp = game.GetViewport();
	const D3D11_RECT fullRect = { 0, 0, (LONG)vp.Width, (LONG)vp.Height };
	context->RSSetScissorRects(1, &fullRect);

	UICB ui{};
	ui.screenW    = vp.Width; 
	ui.screenH    = vp.Height;
	ui.invScreenW = 1.f / max(1.f, ui.screenW);
	ui.invScreenH = 1.f / max(1.f, ui.screenH);
	{
		MinimapScreenRect rect{};
		bool hasMini = false;
		{
			const auto& instances = uiRegistry->GetInstances();
			if (instances.count(L"field_minimap_in"))
			{
				rect = fieldMini->GetMinimapScreenRect(L"field_minimap_in");
				hasMini = true;
			}
		}

		if (hasMini)
		{
			ui.minimapCenter     = { rect.centerX, rect.centerY };
			ui.minimapRadius     = rect.radiusPx;
			ui.minimapMaskEnable = 1.0f;
		}
		else
		{
			ui.minimapCenter     = { 0.f, 0.f };
			ui.minimapRadius     = 0.0f;
			ui.minimapMaskEnable = 0.0f;
		}
	}

	cb.UpdateBindT(CBufferID::UI, ui, SHADER::VS | SHADER::PS, CBUFFERSLOT::UI);
	GetRendererShader(RendererShader::UI)->Bind(context);
	uiMesh->Bind(context, *cb.Get(CBufferID::UI), ui);

	auto resolve = [&](const wstring& key) -> ID3D11ShaderResourceView* 
		{
			if (key == L"field_minimap_in")
				return minimap->GetColorSRV();

			if (key == L"dressing_char_rt")
				return rtSys->GetDressingSRV();

			auto tex = assets->GetTexture(key);
			return tex ? tex->GetSrv() : nullptr;
		};

	uiMesh->Draw(context, items, resolve);

	SetRasterizerState(RASTERIZER::CULLBACK);
	SetBlendState(BLENDSTATE::Opaque);
	SetDepthState(DEPTHSTATE::DEFAULT);
}

void Renderer::DrawGrid()
{
	BindGridState();
	SetBlendState(BLENDSTATE::Opaque);
	gridSys->RenderAllLines(context);
	SetBlendState(BLENDSTATE::ALPHABLEND);
	gridSys->RenderAllHover(context);

	SetDepthState(DEPTHSTATE::DEFAULT);
	SetBlendState(BLENDSTATE::Opaque);
	SetRasterizerState(RASTERIZER::CULLBACK);
}

void Renderer::DrawDebugGBuffers()
{
	ID3D11RenderTargetView* bb = GAME.GetDevicePtr()->GetBackBufferRTV();
	context->OMSetRenderTargets(1, &bb, nullptr); 

	const float tileW = 300.f;
	const float tileH = 300.f;

	D3D11_VIEWPORT oldVP{};
	UINT vpCount = 1;
	context->RSGetViewports(&vpCount, &oldVP);

	SetBlendState(BLENDSTATE::Opaque);
	SetDepthState(DEPTHSTATE::NO_DEPTHTEST);
	SetRasterizerState(RASTERIZER::CULLNONE);

	auto shader = assets->GetShader(L"Debug");
	shader->Bind(context);

	BindSamplers(SHADER::PS, (TEXSLOT)0, SAMPLER::LINEAR);

	DebugCB dbg{};
	dbg.scale = 1.0f;
	dbg.mode  = 0;
	cb.UpdateBindT(CBufferID::Debug, dbg, SHADER::PS, CBUFFERSLOT::Debug);

	auto drawTile = [&](float vx, float vy, ID3D11ShaderResourceView* srv, int mode)
		{
			D3D11_VIEWPORT vp{};
			vp.TopLeftX = vx;
			vp.TopLeftY = vy;
			vp.Width = tileW;
			vp.Height = tileH;
			vp.MinDepth = 0.f;
			vp.MaxDepth = 1.f;
			context->RSSetViewports(1, &vp);

			ID3D11ShaderResourceView* srvs[1] = { srv };
			context->PSSetShaderResources(0, 1, srvs);

			dbg.mode = mode;
			cb.UpdateBindT(CBufferID::Debug, dbg, SHADER::PS, CBUFFERSLOT::Debug);

			DrawFullscreenTriangle();

			ID3D11ShaderResourceView* nulls[1] = {};
			context->PSSetShaderResources(0, 1, nulls);
		};

	// 타일 3개: RT0, RT1, RT2
	drawTile(10.f,         10.f,  rtSys->GetGBufferSRV(GBufferSlot::RT0), 0); // Albedo
	drawTile(10.f + 310.f, 10.f,  rtSys->GetGBufferSRV(GBufferSlot::RT1), 1); // Normal.xy 시각화
	drawTile(10.f + 620.f, 10.f,  rtSys->GetGBufferSRV(GBufferSlot::RT2), 2); // SpecColor
	drawTile(10.f,         320.f, rtSys->GetDepthSRV(),                   3); // Depth(raw)
	drawTile(10.f + 310.f, 320.f, rtSys->GetDepthSRV(),                   4); // Depth(linear)
	drawTile(10.f + 620.f, 320.f, rtSys->GetLightAccumSRV(),              5);
	context->RSSetViewports(1, &oldVP);
}

void Renderer::DrawColliders(const RenderScene& scene)
{
	if (!scene.drawColliders) return;
	if (scene.colliders.empty()) return;

	vector<VertexColor> lines;
	lines.reserve(scene.colliders.size() * 64);

	static const int edges[12][2] =
	{
		{ 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 },
		{ 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 },
		{ 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 }
	};

	const _float4 aabbColor{ 0.f, 1.f, 0.f, 1.f }; // 녹색
	const _float4 obbColor{ 0.f, 1.f, 1.f, 1.f }; // 청록
	const _float4 sphereColor{ 1.f, 1.f, 0.f, 1.f }; // 노랑

	for (const auto& c : scene.colliders)
	{
		switch (c.type)
		{
		case ColliderType::AABB:
		{
			_float3 corners[8];
			c.aabb.GetCorners(corners);

			for (int i = 0; i < 12; ++i)
			{
				const _float3& p0 = corners[edges[i][0]];
				const _float3& p1 = corners[edges[i][1]];
				lines.push_back(VertexColor{ p0, aabbColor });
				lines.push_back(VertexColor{ p1, aabbColor });
			}
		}
		break;

		case ColliderType::OBB:
		{
			_float3 corners[8];
			c.obb.GetCorners(corners);

			for (int i = 0; i < 12; ++i)
			{
				const _float3& p0 = corners[edges[i][0]];
				const _float3& p1 = corners[edges[i][1]];
				lines.push_back(VertexColor{ p0, obbColor });
				lines.push_back(VertexColor{ p1, obbColor });
			}
		}
		break;

		case ColliderType::Sphere:
		{
			const int segments = 24;
			const float r = c.sphereRadius;
			const _float3 center = c.sphereCenter;

			for (int i = 0; i < segments; ++i)
			{
				float t0 = (float)i / segments * XM_2PI;
				float t1 = (float)(i + 1) / segments * XM_2PI;

				// XY
				{
					_float3 p0{
						center.x + cosf(t0) * r,
						center.y + sinf(t0) * r,
						center.z
					};
					_float3 p1{
						center.x + cosf(t1) * r,
						center.y + sinf(t1) * r,
						center.z
					};
					lines.push_back(VertexColor{ p0, sphereColor });
					lines.push_back(VertexColor{ p1, sphereColor });
				}

				// YZ
				{
					_float3 p0{
						center.x,
						center.y + cosf(t0) * r,
						center.z + sinf(t0) * r
					};
					_float3 p1{
						center.x,
						center.y + cosf(t1) * r,
						center.z + sinf(t1) * r
					};
					lines.push_back(VertexColor{ p0, sphereColor });
					lines.push_back(VertexColor{ p1, sphereColor });
				}

				// ZX
				{
					_float3 p0{
						center.x + cosf(t0) * r,
						center.y,
						center.z + sinf(t0) * r
					};
					_float3 p1{
						center.x + cosf(t1) * r,
						center.y,
						center.z + sinf(t1) * r
					};
					lines.push_back(VertexColor{ p0, sphereColor });
					lines.push_back(VertexColor{ p1, sphereColor });
				}
			}
		}
		break;

		default:
			break;
		}
	}

	if (lines.empty()) return;

	SetDepthState(DEPTHSTATE::NO_DEPTHWRITE_LESSEQUAL);
	SetBlendState(BLENDSTATE::ALPHABLEND);
	SetRasterizerState(RASTERIZER::CULLBACK);

	DrawLineList(lines);

	SetDepthState(DEPTHSTATE::DEFAULT);
	SetBlendState(BLENDSTATE::Opaque);
	SetRasterizerState(RASTERIZER::CULLBACK);
}

void Renderer::UpdateCBuffers(const RenderScene& scene)
{
	cb.UpdateBindT(CBufferID::Camera, scene.cam, SHADER::VS | SHADER::PS, CBUFFERSLOT::CAMERA);
	
	if (!scene.lights.empty())
		cb.UpdateBindT(CBufferID::Light, scene.lights[0], SHADER::VS | SHADER::PS, CBUFFERSLOT::LIGHT);

	//TessellationCB ts;
	//cb.UpdateBindT(CBufferID::Tess, ts, SHADER::HS, CBUFFERSLOT::TS);
	
	MaterialCB mtrl;
	cb.UpdateBindT(CBufferID::Material, mtrl, SHADER::PS, CBUFFERSLOT::MATERIAL);

	PostCB post;
	cb.UpdateBindT(CBufferID::Post, post, SHADER::PS, CBUFFERSLOT::POST);

	DistortionCB distort;
	distortSys->ExtractDistortionProxies(distort);
	cb.UpdateBindT(CBufferID::Distortion, distort, SHADER::PS, CBUFFERSLOT::Distortion);
}

void Renderer::BindGridState()
{
	GetRendererShader(RendererShader::Grid)->Bind(context);

	ObjCB obj{};
	_mat I = XMMatrixIdentity();
	XMStoreFloat4x4(&obj.world, I);
	XMStoreFloat4x4(&obj.invWorld, XMMatrixInverse(nullptr, I));
	cb.UpdateBindT(CBufferID::Obj, obj, SHADER::VS, CBUFFERSLOT::OBJ);

	SetRasterizerState(RASTERIZER::CULLNONE);
	SetDepthState(DEPTHSTATE::NO_DEPTHWRITE);
}

HRESULT Renderer::DrawLineList(const vector<VertexColor>& vertexColor)
{
	if (vertexColor.empty()) return S_OK;

	const UINT bytes = (UINT)vertexColor.size() * sizeof(VertexColor);
	GpuUtil::EnsureDynamicBuffer(device, aabbVB.GetAddressOf(), bytes, aabbCapBytes, D3D11_BIND_VERTEX_BUFFER);
	GpuUtil::UploadDynamicBuffer(context, aabbVB.Get(), vertexColor.data(), bytes);

	const UINT stride = sizeof(VertexColor), offset = 0;
	ID3D11Buffer* buf = aabbVB.Get();
	context->IASetVertexBuffers(0, 1, &buf, &stride, &offset);
	context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	auto sh = GetRendererShader(RendererShader::Grid);
	sh->Bind(context);
	context->Draw((UINT)vertexColor.size(), 0);
	return S_OK;
}

HRESULT Renderer::DrawTriList(const vector<VertexColor>& vertices)
{
	if (vertices.empty()) return S_OK;

	const UINT bytes = (UINT)vertices.size() * sizeof(VertexColor);
	GpuUtil::EnsureDynamicBuffer(device, aabbVB.GetAddressOf(), bytes, aabbCapBytes, D3D11_BIND_VERTEX_BUFFER);
	GpuUtil::UploadDynamicBuffer(context, aabbVB.Get(), vertices.data(), bytes);

	const UINT stride = sizeof(VertexColor);
	const UINT offset = 0;
	ID3D11Buffer* buf = aabbVB.Get();
	context->IASetVertexBuffers(0, 1, &buf, &stride, &offset);
	context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	auto sh = GetRendererShader(RendererShader::Grid);
	sh->Bind(context);

	context->Draw((UINT)vertices.size(), 0);
	return S_OK;
}

void Renderer::DrawAABBLines(const BoundingBox& worldAABB, const _float4& color)
{
	_float3 c[8];
	worldAABB.GetCorners(c);

	static const int E[12][2] =
	{
		{ 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 },
		{ 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 },
		{ 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 }
	};

	VertexColor v[24];
	for (int i = 0; i < 12; ++i) { v[i * 2 + 0] = { c[E[i][0]], color }; v[i * 2 + 1] = { c[E[i][1]], color }; }
	const UINT bytes = sizeof(v);

	GpuUtil::EnsureDynamicBuffer(device, aabbVB.GetAddressOf(), bytes, aabbCapBytes, D3D11_BIND_VERTEX_BUFFER);
	GpuUtil::UploadDynamicBuffer(context, aabbVB.Get(), v, bytes);

	const UINT stride = sizeof(VertexColor), offset = 0;
	ID3D11Buffer* buffer = aabbVB.Get();
	context->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
	context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	SetDepthState(DEPTHSTATE::NO_DEPTHWRITE_LESSEQUAL);
	SetBlendState(BLENDSTATE::ALPHABLEND);

	GetRendererShader(RendererShader::Grid)->Bind(context);
	context->Draw(24, 0);

	SetDepthState(DEPTHSTATE::DEFAULT);
	SetBlendState(BLENDSTATE::Opaque);
	SetRasterizerState(RASTERIZER::CULLBACK);
}

void Renderer::ApplySkyCull(SkyCull cull)
{
	switch (cull)
	{
	case SkyCull::Back:  SetRasterizerState(RASTERIZER::CULLBACK);  break;
	case SkyCull::Front: SetRasterizerState(RASTERIZER::CULLFRONT); break;
	case SkyCull::None:  SetRasterizerState(RASTERIZER::CULLNONE);  break;
	}
}

void Renderer::ApplySkyBlend(bool transparent, bool pm)
{
	if (!transparent) { SetBlendState(BLENDSTATE::Opaque); return; }
	SetBlendState(pm ? BLENDSTATE::PM_ALPHA : BLENDSTATE::ALPHABLEND);
}

SkyDrawLists Renderer::BuildSkyDrawLists(const vector<SkySubmesh>& subs)
{
	SkyDrawLists lists{};
	lists.opaque.reserve(subs.size());
	lists.alpha.reserve(subs.size());
	for (const auto& s : subs)
	{
		if (!s.mesh || !s.material || !s.mesh->IsRenderable()) continue;
		if (s.queue == SkyQueue::Alpha) lists.alpha.push_back(&s);
		else                             lists.opaque.push_back(&s);
	}
	return lists;
}

void Renderer::DrawFullscreenTriangle()
{
	context->IASetInputLayout(nullptr);
	context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->Draw(3, 0);
}

void Renderer::DrawNavMesh()
{
	SetDepthState(DEPTHSTATE::NO_DEPTHTEST);
	//SetDepthState(DEPTHSTATE::DEFAULT);
	SetRasterizerState(RASTERIZER::CULLNONE);
	SetBlendState(BLENDSTATE::ALPHABLEND);

	vector<VertexColor> tris;
	nav->BuildDebugTriangles(tris);
	if (!tris.empty())
		DrawTriList(tris);

	vector<VertexColor> lines;
	nav->BuildDebugLines(lines);
	if (!lines.empty())
		DrawLineList(lines);
}