#include "Enginepch.h"
#include "TimeMgr.h"

bool GameInstance::inited = false;
HWND g_hWnd;

GameInstance::GameInstance(PassKey)
	:entityMgr(registry), tfSys(registry), animatorSys(registry), camSys(registry),lightSys(registry),freeCamSys(registry),faceSys(registry), mouthSys(registry),socketSys(registry), modelSys(registry), layerSys(registry), gridSys(registry), pickingSys(registry), selectionSys(registry), collisionSys(registry), renderSys(registry) {}
GameInstance::~GameInstance() = default;
HRESULT GameInstance::InitEngine(const EngineDesc& _engineDesc)
{
	g_hWnd      = _engineDesc.hWnd;

	DeviceOptions opts;
	device      = Device::Create(_engineDesc.winMode, opts);

	timeMgr     = TimeMgr::Create();
	levelMgr    = LevelMgr::Create();
	inputMgr    = InputMgr::Create();
// -------------------------------------
	assetSys.Init();

	registry.Register(tfSys);
	registry.Register(camSys);
	registry.Register(freeCamSys);
	registry.Register(lightSys);
	registry.Register(animatorSys);
	registry.Register(faceSys);
	registry.Register(mouthSys);
	registry.Register(socketSys);
	registry.Register(modelSys);
	registry.Register(layerSys);
	registry.Register(tagSys);
	registry.Register(gridSys);
	registry.Register(pickingSys);
	registry.Register(collisionSys);
	registry.Register(assetSys);

	static HighlightSystem highlightSys;
	registry.Register(highlightSys);

	registry.Register(selectionSys);
	registry.Reserve(2048);

// ---------------------------
	renderer    = Renderer::Create();
	guiMgr      = GuiMgr::Create(registry, entityMgr);

	inited = true;
	return S_OK;
}
// Update 순서흐름
// 1. System
// 2. RenderScene (ExtractProxies)
// 3. Draw
// 4. Deferred Destory

void GameInstance::UpdateEngine(float dt)
{
	tfSys.Update(dt);
	animatorSys.Update(dt, tfSys);
	socketSys.Update(dt);
	faceSys.Update(dt);
	camSys.Update(dt);
	freeCamSys.Update(dt);
	lightSys.Update(dt);
	pickingSys.Update(dt);
	selectionSys.Update(dt);
	gridSys.Update(dt);
	collisionSys.Update(dt);

	levelMgr->Update(dt);
}

HRESULT GameInstance::BeginDraw(const _float4 color)
{
	device->ClearBackBufferView(color);
	device->ClearDSV();

	return S_OK;
}

HRESULT GameInstance::Draw()
{
	// Local Static 변수라 한번 할당되고 RenderScene 은 풀링되는중 [프레임간 메모리 풀링중]
	static RenderScene scene;
	renderSys.BuildScene(scene);

	renderer->Draw(scene);
	levelMgr->Render();
	entityMgr.FlushDestroy();

	return S_OK;
}

HRESULT GameInstance::EndDraw()
{
	HR(device->Present());
	return S_OK;
}

void GameInstance::ClearResources(_uint levelID)
{
}

void GameInstance::ReleaseEngine()
{
	ClearEntities();
	registry.Clear();
	device->ReleaseDevice();
	inited = false;
}

void GameInstance::BeginFrame(float dt)
{
	auto& highlightSys = registry.Get<HighlightSystem>();
	highlightSys.ClearFrame();

	guiMgr->Update(dt);
	inputMgr->BeginFrame();
}

void GameInstance::EndFrame()
{
	inputMgr->EndFrame();
}
// ------------- EntityMgr --------------
EntityID GameInstance::CreateEntity()
{
	return entityMgr.Create();
}

void GameInstance::DestroyEntity(EntityID id)
{
	entityMgr.Destroy(id);
}

void GameInstance::DestroyEntityDeferred(EntityID id)
{
	entityMgr.DestroyDeferred(id);
}

void GameInstance::FlushDestroyedEntities()
{
	entityMgr.FlushDestroy();
}

bool GameInstance::IsEntityAlive(EntityID id) const
{
	return entityMgr.IsAlive(id);
}

void GameInstance::ReserveEntities(size_t n)
{
	entityMgr.Reserve(n);
}

void GameInstance::ClearEntities()
{
	entityMgr.Clear();
}

// ----------------------------Device ------------------------
const D3D11_VIEWPORT& GameInstance::GetViewport() const
{
	return device->GetViewport();
}

ID3D11Device* GameInstance::GetDevice() const
{
	return device->GetDevice();
}

ID3D11DeviceContext* GameInstance::GetContext() const
{
	return device->GetContext();
}

void GameInstance::OnResize(_uint newX, _uint newY)
{
	device->OnResize(newX, newY);
}

ID3D11RenderTargetView* GameInstance::GetBackBufferRTV() const
{
	return device->GetBackBufferRTV();
}

ID3D11DepthStencilView* GameInstance::GetDSV() const
{
	return device->GetDSV();
}

ID3D11ShaderResourceView* GameInstance::GetDepthSRV() const
{
	return device->GetDepthSRV();
}

// --------------------------- TimeMgr --------------------------
_float GameInstance::GetDt(TIMER timerID)
{
	return timeMgr->GetDt(timerID);
}

void GameInstance::UpdateDt(TIMER timerID)
{
	timeMgr->UpdateDt(timerID);
}

void GameInstance::ChangeLevel(_uint levelID, unique_ptr<Level> newLevel)
{
	return levelMgr->ChangeLevel(levelID, move(newLevel));
}

_uint GameInstance::GetCurLevelID()
{
	return levelMgr->GetCurLevelID();
}

// ---------------------- InputMgr--------------------------------------

void GameInstance::ProcessWinMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
	inputMgr->ProcessWinMsg(msg, wParam, lParam);
}

bool GameInstance::KeyPressing(KEY key)
{
	return inputMgr->KeyPressing(key);
}

bool GameInstance::KeyDown(KEY key)
{
	return inputMgr->KeyDown(key);
}

bool GameInstance::KeyRelease(KEY key)
{
	return inputMgr->KeyRelease(key);
}

const _float2& GameInstance::GetMouseDelta() const
{
	return inputMgr->GetMouseDelta();
}

const _float2& GameInstance::GetMousePos() const
{
	return inputMgr->GetMousePos();
}
// ---------------- Renderer -------------------------------------
void GameInstance::BindSamplers(SHADER stage, TEXSLOT slot, SAMPLER type)
{
	renderer->BindSamplers(stage, slot, type);
}

void GameInstance::SetRasterizerState(RASTERIZER type)
{
	renderer->SetRasterizerState(type);
}

void GameInstance::SetDepthState(DEPTHSTATE type)
{
	renderer->SetDepthState(type);
}

void GameInstance::SetBlendState(BLENDSTATE type)
{
	renderer->SetBlendState(type);
}

// -------------- Imgui -------------------
LRESULT GameInstance::ImguiWndProcHandler(_uint msg, WPARAM wParam, LPARAM lParam)
{
	return guiMgr->ImguiWndProcHandler(msg, wParam, lParam);
}

void GameInstance::GuiRender()
{
	guiMgr->Render();
}