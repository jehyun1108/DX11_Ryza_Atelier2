#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL Level abstract
{
protected:
	Level();

public:
	virtual ~Level() = default;

public:
	virtual HRESULT Init()           = 0;
	virtual void    Update(float dt) = 0;
	virtual void    Render()         = 0;

protected:
	GameInstance& game = GameInstance::GetInstance();
	ID3D11Device*           device{};
	ID3D11DeviceContext*    context{};
	SystemRegistry&         registry;
						    
	EntityMgr*              entityMgr{};
	EntitySpawner*          spawner{};
	AssetSystem*            assets{};
	GridSystem*             gridSys{};
	CameraSystem*           camSys{};
	TagSystem*              tagSys{};
	LayerSystem*            layerSys{};
	TransformSystem*        tfSys{};
	ModelSystem*            modelSys{};
	PickingSystem*          pickSys{};
	SelectionSystem*        selectSys{};
	CollisionSystem*        collisionSys{};
	InputService*           input{};
	FieldAnimSystem*        fieldAnimSys{};
	FacingSystem*           faceSys{};
	FieldControllerSystem*  fieldCtrlSys{};
	CharacterDataSystem*    dataSys{};
	UIRegistry*             uiRegistry{};
	NavMeshSystem*          navSys{};
	WorldSerializer*        worldSys{};
	GameModeDirectorSystem* director{};
	SoundSystem*            soundSys{};
	SoundRegistry*          soundRegistry{};
	UISystem*               uiSys{};
	LoadingPresenter*       loadPresenter{};
	ScreenFadeSystem*       fadeSys{};
	LoadingPresenter*       loadingPresenter{};
	LogoMenuPresenter*      logoMenuPresenter{};
	ParticleSystem*         particleSys{};
	EffectSystem*           effectSys{};
	ActionFxRegistry*       actionFxReg{};

	EntityID                playerID{};
	EntityHandles           playerHandle{};
};

NS_END