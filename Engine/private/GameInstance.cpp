#include "Enginepch.h"
#include "TimeMgr.h"

bool GameInstance::inited = false;
HWND g_hWnd;

GameInstance::GameInstance(PassKey)
	:entityMgr(registry), tfSys(registry), animatorSys(registry), camSys(registry), lightSys(registry), freeCamSys(registry), faceSys(registry), mouthSys(registry), socketSys(registry), modelSys(registry), layerSys(registry), gridSys(registry), pickingSys(registry), selectionSys(registry), collisionSys(registry), renderSys(registry), moveStateSys(registry), moveProfileSys(registry), moveIntentSys(registry), meshColliderSys(registry), skySys(registry), fieldAnimSys(registry), facingSys(registry),  orbitCamSys(registry), fieldCtrlSys(registry), battleIntroSys(registry), animDataSys(registry), jumpSys(registry), battleSessionSys(registry), fieldSys(registry), battleSys(registry), battleCtrlSys(registry) ,charaDataSys(registry), battleTimelineSys(registry), battleExecSys(registry) {
}
GameInstance::~GameInstance() = default;
HRESULT GameInstance::InitEngine(const EngineDesc& _engineDesc)
{
	DeviceOptions opts;
	g_hWnd      = _engineDesc.hWnd;
	device      = Device::Create(_engineDesc.winMode, opts);
	timeMgr     = TimeMgr::Create();

	assetSys.Init();

	registry.Register(tfSys);
	registry.Register(fieldAnimSys);
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
	registry.Register(moveStateSys);
	registry.Register(moveProfileSys);
	registry.Register(moveIntentSys);
	registry.Register(meshColliderSys);
	registry.Register(selectionSys);
	registry.Register(skySys);
	registry.Register(orbitCamSys);
	registry.Register(fieldCtrlSys);
	registry.Register(battleCtrlSys);
	registry.Register(battleIntroSys);
	registry.Register(animDataSys);
	registry.Register(jumpSys);
	registry.Register(facingSys);
	registry.Register(faceBlockSrv);
	registry.Register(faceForceSrv);
	registry.Register(battleSessionSys);
	registry.Register(animRegistry);
	registry.Register(battleSys);
	registry.Register(fieldSys);
	registry.Register(charaDataSys);
	registry.Register(battleCtrlSys);
	registry.Register(battleTimelineSys);
	registry.Register(battleExecSys);
	
	animDataSys.RegisterDefaultClips();
	animRegistry.RegisterDefaultAnim();

	// --------------------------------
	static HighlightSystem highlightSys;
	registry.Register(highlightSys);
	registry.Register(inputService);

	static GameModeDirectorSystem director{ registry, 1 };
	registry.Register(director);

	// ----------------------------------
	registry.Reserve(1024);

	levelMgr    = LevelMgr::Create();
	input       = InputMgr::Create();
	renderer    = Renderer::Create();
	guiMgr      = GuiMgr::Create(registry, entityMgr);
// ---------------------------
	inited = true;
	return S_OK;
}
// Update 순서흐름
// 1. System
// 2. RenderScene (ExtractProxies)
// 3. Draw
// 4. Deferred Destory
// Channel -> Buffer -> SnapShot -> Merge -> 한번만 적용
void GameInstance::UpdateEngine(float dt)
{
	// 0. Level 전용 
	levelMgr->Update(dt);
	// 1. Input Frame 시작 (쿨다운 등 시간 경과)
	inputService.BeginFrame(dt);
	registry.Get<GameModeDirectorSystem>().Update(dt);
	// 3. Frame 말에 "한번만" Intent Merge & 적용 -> Collector 비움
	inputService.EndFrameAndApply(registry);
	jumpSys.Priority_Update(dt);
	// 4. 이동/물리 -> Transform -> 
	moveStateSys.Update(dt);
	tfSys.Update(dt);
	// 5. AnimSys
	facingSys.Update(dt);
	animatorSys.Update(dt, tfSys);
	// 6. 기타
	socketSys.Update(dt);
	faceSys.Update(dt);
	orbitCamSys.Update(dt);
	camSys.Update(dt);
	freeCamSys.Update(dt);
	lightSys.Update(dt);
	//pickingSys.Update(dt);
	//selectionSys.Update(dt);
	gridSys.Update(dt);
	collisionSys.Update(dt);

	skySys.Tick(dt);
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
	input->BeginFrame();
}

void GameInstance::EndFrame()
{
	input->EndFrame();
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
	input->ProcessWinMsg(msg, wParam, lParam);
}

bool GameInstance::KeyPressing(KEY key)
{
	return input->KeyPressing(key);
}

bool GameInstance::KeyDown(KEY key)
{
	return input->KeyDown(key);
}

bool GameInstance::KeyRelease(KEY key)
{
	return input->KeyRelease(key);
}

const _float2& GameInstance::GetMouseDelta() const
{
	return input->GetMouseDelta();
}

const _float2& GameInstance::GetMousePos() const
{
	return input->GetMousePos();
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