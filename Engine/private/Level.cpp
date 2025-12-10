#include "Enginepch.h"

Level::Level() : registry(game.GetRegistry()) 
{
	device  = game.GetDevice();
	context = game.GetContext();

	entityMgr         = &registry.Get<EntityMgr>();
	spawner           = &registry.Get<EntitySpawner>();
	assets            = &registry.Get<AssetSystem>();
	gridSys           = &registry.Get<GridSystem>();
	camSys            = &registry.Get<CameraSystem>();
	tagSys            = &registry.Get<TagSystem>();
	layerSys          = &registry.Get<LayerSystem>();
	tfSys             = &registry.Get<TransformSystem>();
	modelSys          = &registry.Get<ModelSystem>();
	pickSys           = &registry.Get<PickingSystem>();
	selectSys         = &registry.Get<SelectionSystem>();
	collisionSys      = &registry.Get<CollisionSystem>();
	input             = &registry.Get<InputService>();
	fieldAnimSys      = &registry.Get<FieldAnimSystem>();
	faceSys           = &registry.Get<FacingSystem>();
	fieldCtrlSys      = &registry.Get<FieldControllerSystem>();
	dataSys           = &registry.Get<CharacterDataSystem>();
	uiRegistry        = &registry.Get<UIRegistry>();
	navSys            = &registry.Get<NavMeshSystem>();
	worldSys          = &registry.Get<WorldSerializer>();
	director          = &registry.Get<GameModeDirectorSystem>();
	soundSys          = &registry.Get<SoundSystem>();
	soundRegistry     = &registry.Get<SoundRegistry>();
	uiSys             = &registry.Get<UISystem>();
	loadPresenter     = &registry.Get<LoadingPresenter>();
	fadeSys           = &registry.Get<ScreenFadeSystem>();
	loadingPresenter  = &registry.Get<LoadingPresenter>();
	logoMenuPresenter = &registry.Get<LogoMenuPresenter>();
	particleSys       = &registry.Get<ParticleSystem>();
	effectSys         = &registry.Get<EffectSystem>();
	actionFxReg       = &registry.Get<ActionFxRegistry>();
}