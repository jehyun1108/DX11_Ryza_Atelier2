#include "Enginepch.h"
#include "TimeMgr.h"
#include "JumpSystem.h"
#include "BattleTimelinePresenter.h"
#include "PlayerInputPresenter.h"
#include "BattleUIOrchestrator.h"
#include "BattleEventBus.h"
#include "BattleSessionSystem.h"
#include "BattleHUDPresenter.h"
#include "BattleTacticSystem.h"
#include "BattleFatalDrivePresenter.h"
#include "BattleDamagePresenter.h"
#include "BattleAttributeSystem.h"
#include "WorldSerializer.h"
#include "NavMeshSystem.h"
#include "LogoOrchestraSystem.h"
#include "LogoUIOrchestrator.h"
#include "LogoMenuPresenter.h"
#include "RenderTargetMinimap.h"
#include "FieldMinimapPresenter.h"
#include "BattleMinimapPresenter.h"
#include "UIMinimapSystem.h"
#include "WorldMapPresenter.h"
#include "ScreenFadeSystem.h"
#include "LoadingPresenter.h"
#include "ScreenDistortionSystem.h"
#include "EffectSerializer.h"
#include "BattleTargetHUDPresenter.h"
#include "BattleBoardPresenter.h"
#include "DressingRoomPresenter.h"
#include "CamSerializer.h"
#include "ActionFxRegistry.h"
#include "BattleRewardPresenter.h"
#include "SoundSystem.h"
#include "SoundRegistry.h"

bool GameInstance::inited = false;

HWND g_hWnd;

GameInstance::GameInstance(PassKey) {}
GameInstance::~GameInstance() = default;
HRESULT GameInstance::InitEngine(const EngineDesc& _engineDesc)
{
	DeviceOptions opts;
	g_hWnd   = _engineDesc.hWnd;
	device   = Device::Create(_engineDesc.winMode, opts);
	timeMgr  = TimeMgr::Create();
	levelMgr = LevelMgr::Create();

	registry.EmplaceAll<Data>();
	registry.EmplaceAll<Core>();
	registry.EmplaceAll<Scene>();
	registry.EmplaceAll<UI>();
	registry.EmplaceAll<Battle>();
	registry.EmplaceAll<Field>();
	registry.BootAll();
	registry.Reserve(512);

	BootingSystems();
// =====================================================================================
	soundSys->Init();

	device->onResized = [&](_uint width, _uint height) { rtSys->Resize(width, height); };

	const auto& vp = GetViewport();
	GBufferSpec spec;
	rtSys->Init((_uint)vp.Width, (_uint)vp.Height, spec);
// ---------------------------
	inited = true;
	return S_OK;
}

void GameInstance::UpdateEngine(float dt)
{
	levelMgr->Update(dt);

	input->BeginFrame(dt);
	director->Update(dt);

	input->EndFrameAndApply(registry);
	jumpSys->Priority_Update(dt); 

	moveSys->Update(dt);      
	tfSys->Update(dt);

	facingSys->Update(dt);
	animator->Update(dt);

	socketSys->Update(dt);
	effectSys->Tick(dt);
	trailSys->Tick(dt);
	particleSys->Tick(dt);

	faceSys->Update(dt);
	orbitCamSys->Update(dt);
	camSys->Update(dt);
	freeCamSys->Update(dt);
	lightSys->Update(dt);

	//selectSys->Update(dt);
	//gridSys->Update(dt);
	collisionSys->Tick(dt);
	skySys->Tick(dt);
	soundSys->Tick(dt);
}

HRESULT GameInstance::Draw()
{
	static RenderScene scene;
	renderSys->BuildScene(scene);
	renderer->Draw(scene);
	levelMgr->Render();
	entityMgr->FlushDestroy();

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
	soundSys->Shutdown();
	registry.Clear();
	device->ReleaseDevice();
	inited = false;
}

void GameInstance::BeginFrame(float dt)
{
	gameTime += dt;
	guiMgr->Update(dt);

}

void GameInstance::EndFrame()
{

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
// -------------- Imgui -------------------
LRESULT GameInstance::ImguiWndProcHandler(_uint msg, WPARAM wParam, LPARAM lParam)
{
	return guiMgr->ImguiWndProcHandler(msg, wParam, lParam);
}

void GameInstance::GuiRender()
{
	guiMgr->Render();
}

void GameInstance::BootingSystems()
{
	guiMgr        = &registry.Get<GuiMgr>();
	inputMgr      = &registry.Get<InputMgr>();
	renderSys     = &registry.Get<RenderSystem>();
	renderer      = &registry.Get<Renderer>();
	soundSys      = &registry.Get<SoundSystem>();
	entityMgr     = &registry.Get<EntityMgr>();
	soundRegistry = &registry.Get<SoundRegistry>();
	rtSys         = &registry.Get<RenderTargetSystem>();
	input         = &registry.Get<InputService>();
	director      = &registry.Get<GameModeDirectorSystem>();
	jumpSys       = &registry.Get<JumpSystem>();
	moveSys       = &registry.Get<MoveStateSystem>();
	tfSys         = &registry.Get<TransformSystem>();
	facingSys     = &registry.Get<FacingSystem>();
	animator      = &registry.Get<AnimatorSystem>();
	socketSys     = &registry.Get<SocketSystem>();
	faceSys       = &registry.Get<FaceSystem>();
	orbitCamSys   = &registry.Get<OrbitCamSystem>();
	camSys        = &registry.Get<CameraSystem>();
	freeCamSys    = &registry.Get<FreeCamSystem>();
	lightSys      = &registry.Get<LightSystem>();
	gridSys       = &registry.Get<GridSystem>();
	collisionSys  = &registry.Get<CollisionSystem>();
	skySys        = &registry.Get<SkyboxSystem>();
	selectSys     = &registry.Get<SelectionSystem>();
	particleSys   = &registry.Get<ParticleSystem>();
	effectSys     = &registry.Get<EffectSystem>();
	trailSys      = &registry.Get<TrailSystem>();
}