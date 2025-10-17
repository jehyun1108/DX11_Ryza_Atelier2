#pragma once

NS_BEGIN(Engine)
class Level;
class Device;
class LevelMgr;
class TimeMgr;
class Renderer;
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
	HRESULT               BeginDraw(const _float4 color);
	HRESULT               Draw();
	HRESULT               EndDraw();
	void                  ClearResources(_uint levelID);
	void                  ReleaseEngine();
	const D3D11_VIEWPORT& GetViewport() const;
	static bool           IsInited() { return inited; }

	void BeginFrame(float dt);
	void EndFrame();

	// --------------- EntityMgr ---------------
	EntityID CreateEntity();
	void DestroyEntity(EntityID id);
	void DestroyEntityDeferred(EntityID id);
	void FlushDestroyedEntities();

	bool IsEntityAlive(EntityID id) const;
	void ReserveEntities(size_t n);
	void ClearEntities();

	template<typename Fn>
	void ForEachEntity(Fn&& fn) const { entityMgr.ForEachAlive(forward<Fn>(fn)); }

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
	void           ProcessWinMsg(UINT msg, WPARAM wParam, LPARAM lParam);
	bool           KeyPressing(KEY key);
	bool           KeyDown(KEY key);
	bool           KeyRelease(KEY key);
	const _float2& GetMouseDelta() const;
	const _float2& GetMousePos() const;

	// --------------- LevelMgr --------------------------------------------------------------------------------
	void  ChangeLevel(_uint levelID, unique_ptr<Level> newLevel);
	_uint GetCurLevelID();

	// --------------- Renderer ---------------------------------------------------
	void BindSamplers(SHADER stage, TEXSLOT slot, SAMPLER type);
	void SetRasterizerState(RASTERIZER type);
	void SetDepthState(DEPTHSTATE type);
	void SetBlendState(BLENDSTATE type);

	// ------------ Imgui -----------------------------------------------
	LRESULT ImguiWndProcHandler(_uint msg, WPARAM wParam, LPARAM lParam);

	template<typename T, typename...Args>
	T* AddPanel(string title, Args&&... args) { return guiMgr->AddPanel<T>(title); }

	void GuiRender();

	// ----------- System -----------------------------------------------
	SystemRegistry&    GetRegistry()          { return registry; }
	EntityMgr&         GetEntityMgr()         { return entityMgr; }
	AssetSystem&       GetAssetSystem()       { return assetSys; }
	const AssetSystem& GetAssetSystem() const { return assetSys; }

private:
	static bool inited;

	unique_ptr<Device>      device{};
	unique_ptr<TimeMgr>     timeMgr{};
	unique_ptr<InputMgr>    inputMgr{};
	unique_ptr<LevelMgr>    levelMgr{};
	unique_ptr<Renderer>    renderer{};
	unique_ptr<GuiMgr>      guiMgr{};
	unique_ptr<RenderScene> renderScene{};

	// -------------------------------
	EntityMgr       entityMgr;
	SystemRegistry  registry{};
	TransformSystem tfSys;
	CameraSystem    camSys;
	LightSystem     lightSys;
	FreeCamSystem   freeCamSys;
	AnimatorSystem  animatorSys;
	FaceSystem      faceSys;
	MouthSystem     mouthSys;
	SocketSystem    socketSys;
	ModelSystem     modelSys;
	LayerSystem     layerSys;
	TagSystem       tagSys;
	GridSystem      gridSys;
	PickingSystem   pickingSys;
	SelectionSystem selectionSys;
	CollisionSystem collisionSys;
	
	RenderSystem    renderSys;
	AssetSystem     assetSys;
};

NS_END