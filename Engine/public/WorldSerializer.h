#pragma once

#include "WorldInstanceData.h"

NS_BEGIN(Engine)

class ENGINE_DLL WorldSerializer
{
public:
	WorldSerializer(SystemRegistry& registry, EntityMgr& entities) : registry(registry), entities(entities),
		tfSys(registry.Get<TransformSystem>()),
		layerSys(registry.Get<LayerSystem>()),
		modelSys(registry.Get<ModelSystem>()),
		assets(registry.Get<AssetSystem>())
	{
	}
	// 1. ���� ���忡�� Export ��� Entity ���� ����
	bool SaveWorldToFile(const filesystem::path& outPath, const vector<EntityID>& entityList, string& outErrorMsg);

	// 2. ������ �о� Data ����� ����� ���� ����
	bool LoadWorldFromFile(const filesystem::path& inPath, vector<EntityID>& outSpawnedEntities, string& outErrorMsg);

	// 3. DTO ���� API: ���� ~DTO, DTO��ƼƼ �и�
	bool SaveDtoToFile(const filesystem::path& outPath, const vector<WorldInstanceDto>& instances, string& outErrorMsg);
	bool LoadDtoFromFile(const filesystem::path& inPath, vector<WorldInstanceDto>& outInstances, string& outErrorMsg);

private:
	// Entity -> DTO ��ȯ 
	bool BuildDtoFromEntity(EntityID entityId, WorldInstanceDto& outDto);
	// DTO -> Entity Spanw
	EntityID SpawnFromDto(const WorldInstanceDto& dto);

private:
	SystemRegistry& registry;
	EntityMgr& entities;
	TransformSystem& tfSys;
	LayerSystem& layerSys;
	ModelSystem& modelSys;
	AssetSystem& assets;
};

NS_END