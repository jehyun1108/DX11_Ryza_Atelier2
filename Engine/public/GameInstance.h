#pragma once

NS_BEGIN(Engine)
class Level;
class Device;
class LevelMgr;
class TimeMgr;
class Renderer;
class AssetSystem;
struct RenderScene;
class InputService;

class ENGINE_DLL GameInstance final : public Singleton<GameInstance>
{
public:
	GameInstance(PassKey);
	~GameInstance();

	// ------------ Engine ---------------------------------------------------------------------------------
public:
	HRESULT               InitEngine(const EngineDesc& _engineDesc);
	void                  UpdateEngine(float dt);
	HRESULT               BeginDraw(const _float4 color);
	HRESULT               Draw();
	HRESULT               EndDraw();
	void                  ClearResources(_uint levelID);
	void                  ReleaseEngine();
	const D3D11_VIEWPORT& GetViewport() const;
	static bool           IsInited() { return inited; }

	void BeginFrame(float dt);
	void EndFrame();

	// -------------- Device ---------------------------------------------------------------------------------
	ID3D11Device*             GetDevice() const;
	ID3D11DeviceContext*      GetContext() const;
	void                      OnResize(_uint newX, _uint newY);
	ID3D11RenderTargetView*   GetBackBufferRTV() const;
	ID3D11DepthStencilView*   GetDSV() const;
	ID3D11ShaderResourceView* GetDepthSRV() const;

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
	static bool             inited;
	SystemRegistry          registry;

	unique_ptr<Device>      device{};
	unique_ptr<TimeMgr>     timeMgr{};
	unique_ptr<LevelMgr>    levelMgr{};
};

NS_END