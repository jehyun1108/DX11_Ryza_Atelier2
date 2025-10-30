#include "Enginepch.h"

Level::Level()
	:registry(game.GetRegistry()), 
	entityMgr(game.GetEntityMgr()), 
	spawner(registry, entityMgr, game.GetAssetSystem()), 
	assets(game.GetAssetSystem()),
	gridSys(registry.Get<GridSystem>()),
	camSys(registry.Get<CameraSystem>()),
	tagSys(registry.Get<TagSystem>()),
	layerSys(registry.Get<LayerSystem>()),
	tfSys(registry.Get<TransformSystem>()),
	modelSys(registry.Get<ModelSystem>()),
	pickSys(registry.Get<PickingSystem>()),
	selectSys(registry.Get<SelectionSystem>()),
	collisionSys(registry.Get<CollisionSystem>()),
	inputSerivce(game.GetInputService()),
	fieldAnimSys(registry.Get<FieldAnimSystem>()),
	faceAnimSys(registry.Get<FacingSystem>()),
	fieldCtrlSys(registry.Get<FieldControllerSystem>()),
	charaDataSys(registry.Get<CharacterDataSystem>())
{
	device = game.GetDevice();
	context = game.GetContext();
}