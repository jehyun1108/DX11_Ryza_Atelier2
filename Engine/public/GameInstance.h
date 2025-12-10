#pragma once

NS_BEGIN(Engine)
class Level;
class Device;
class LevelMgr;
class TimeMgr;
struct RenderScene;

class ENGINE_DLL GameInstance final : public Singleton<GameInstance>
{
public:
	GameInstance(PassKey);
	~GameInstance();

	// ------------ Engine ---------------------------------------------------------------------------------
public:
	HRESULT               InitEngine(const EngineDesc& _engineDesc);
	void                  UpdateEngine(float dt);
	HRESULT               Draw();

	HRESULT               EndDraw();
	void                  ClearResources(_uint levelID);
	void                  ReleaseEngine();
	const D3D11_VIEWPORT& GetViewport() const;
	static bool           IsInited()   { return inited; }

	float GetGameTime() const { return gameTime; }

	void BeginFrame(float dt);
	void EndFrame();

	// -------------- Device ---------------------------------------------------------------------------------
	Device*                   GetDevicePtr() const { return device.get(); }
	ID3D11Device*             GetDevice() const;
	ID3D11DeviceContext*      GetContext() const;
	void                      OnResize(_uint newX, _uint newY);
	ID3D11RenderTargetView*   GetBackBufferRTV() const;

	// -------------- TimeMgr ---------------------------------------------------------------------------------
	_float GetDt(TIMER timerID);
	void   UpdateDt(TIMER timerID);

	// --------------- InputMgr --------------------------------------------------------------------------------
	void  ProcessWinMsg(UINT msg, WPARAM wParam, LPARAM lParam);

	// --------------- LevelMgr --------------------------------------------------------------------------------
	void  ChangeLevel(_uint levelID, unique_ptr<Level> newLevel);
	_uint GetCurLevelID();
	// ------------ Imgui -----------------------------------------------
	LRESULT ImguiWndProcHandler(_uint msg, WPARAM wParam, LPARAM lParam);

	template<typename T, typename...Args>
	T* AddPanel(string title, Args&&... args) { return registry.Get<GuiMgr>().AddPanel<T>(title); }

	void GuiRender();

	// ----------- System -----------------------------------------------
	SystemRegistry&    GetRegistry()          { return registry; }

private:
	void BootingSystems();

private:
	static bool             inited;
	float                   gameTime = 0.f;

	SystemRegistry          registry;
	unique_ptr<Device>      device{};
	unique_ptr<TimeMgr>     timeMgr{};
	unique_ptr<LevelMgr>    levelMgr{};

private:
	GuiMgr*                 guiMgr{};
	InputMgr*               inputMgr{};
	RenderSystem*           renderSys{};
	Renderer*               renderer{};
	SoundSystem*            soundSys{};
	EntityMgr*              entityMgr{};
	SoundRegistry*          soundRegistry{};
	RenderTargetSystem*     rtSys{};
	InputService*           input{};
	GameModeDirectorSystem* director{};
	JumpSystem*             jumpSys{};
	MoveStateSystem*        moveSys{};
	TransformSystem*        tfSys{};
	FacingSystem*           facingSys{};
	AnimatorSystem*         animator{};
	SocketSystem*           socketSys{};
	FaceSystem*             faceSys{};
	OrbitCamSystem*         orbitCamSys{};
	CameraSystem*           camSys{};
	FreeCamSystem*          freeCamSys{};
	LightSystem*            lightSys{};
	GridSystem*             gridSys{};
	CollisionSystem*        collisionSys{};
	SkyboxSystem*           skySys{};
	SelectionSystem*        selectSys{};
	ParticleSystem*         particleSys{};
	EffectSystem*           effectSys{};
	TrailSystem*            trailSys{};
};

NS_END