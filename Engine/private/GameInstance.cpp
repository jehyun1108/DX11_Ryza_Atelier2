#include "Enginepch.h"
#include "TimeMgr.h"

bool GameInstance::inited = false;
HWND g_hWnd;

GameInstance::GameInstance(PassKey) {}
GameInstance::~GameInstance() = default;
HRESULT GameInstance::InitEngine(const EngineDesc& _engineDesc)
{
	DeviceOptions opts;
	g_hWnd      = _engineDesc.hWnd;
	device      = Device::Create(_engineDesc.winMode, opts);
	timeMgr     = TimeMgr::Create();
	levelMgr    = LevelMgr::Create();

	registry.EmplaceAll<Data>();
	registry.EmplaceAll<Core>();
	registry.EmplaceAll<Scene>();
	registry.EmplaceAll<UI>();
	registry.EmplaceAll<Battle>();
	registry.EmplaceAll<Field>();
	registry.BootAll();

	registry.Reserve(1024);
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
	registry.Get<InputService>().BeginFrame(dt);
	registry.Get<GameModeDirectorSystem>().Update(dt);
	// 3. Frame 말에 "한번만" Intent Merge & 적용 -> Collector 비움
	registry.Get<InputService>().EndFrameAndApply(registry);
	registry.Get<JumpSystem>().Priority_Update(dt);
	// 4. 이동/물리 -> Transform -> 
	registry.Get<MoveStateSystem>().Update(dt);
	registry.Get<TransformSystem>().Update(dt);
	// 5. AnimSys
	registry.Get<FacingSystem>().Update(dt);
	registry.Get<AnimatorSystem>().Update(dt, registry.Get<TransformSystem>());
	// 6. 기타
	registry.Get<SocketSystem>().Update(dt);
	registry.Get<FaceSystem>().Update(dt);
	registry.Get<OrbitCamSystem>().Update(dt);
	registry.Get<CameraSystem>().Update(dt);
	registry.Get<FreeCamSystem>().Update(dt);
	registry.Get<LightSystem>().Update(dt);
	//pickingSys.Update(dt);
	//selectionSys.Update(dt);
	registry.Get<GridSystem>().Update(dt);
	registry.Get<CollisionSystem>().Update(dt);
	registry.Get<SkyboxSystem>().Tick(dt);
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
	registry.Get<RenderSystem>().BuildScene(scene);

	registry.Get<Renderer>().Draw(scene);
	levelMgr->Render();
	registry.Get<EntityMgr>().FlushDestroy();

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
	registry.Clear();
	device->ReleaseDevice();
	inited = false;
}

void GameInstance::BeginFrame(float dt)
{
	registry.Get<HighlightSystem>().ClearFrame();

	registry.Get<GuiMgr>().Update(dt);
	registry.Get<InputMgr>().BeginFrame();
}

void GameInstance::EndFrame()
{
	registry.Get<InputMgr>().EndFrame();
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
	registry.Get<InputMgr>().ProcessWinMsg(msg, wParam, lParam);
}
// -------------- Imgui -------------------
LRESULT GameInstance::ImguiWndProcHandler(_uint msg, WPARAM wParam, LPARAM lParam)
{
	return registry.Get<GuiMgr>().ImguiWndProcHandler(msg, wParam, lParam);
}

void GameInstance::GuiRender()
{
	registry.Get<GuiMgr>().Render();
}