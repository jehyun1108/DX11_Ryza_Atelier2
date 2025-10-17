#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL Level abstract
{
protected:
	Level();

public:
	virtual ~Level() = default;

public:
	virtual HRESULT Init() = 0;
	virtual void Update(float dt) = 0;
	virtual void Render() = 0;

protected:
	ID3D11Device*    device{};
	ID3D11DeviceContext* context{};
	GameInstance&    game = GameInstance::GetInstance();
	SystemRegistry&  registry;
	EntityMgr&       entityMgr;
	EntitySpawner    spawner;
	AssetSystem&     assets;
	GridSystem&      gridSys;
	CameraSystem&    camSys;
	TagSystem&       tagSys;
	LayerSystem&     layerSys;
	TransformSystem& tfSys;
	ModelSystem&     modelSys;
	PickingSystem&   pickSys;
	SelectionSystem& selectSys;
	CollisionSystem& collisionSys;
};

NS_END