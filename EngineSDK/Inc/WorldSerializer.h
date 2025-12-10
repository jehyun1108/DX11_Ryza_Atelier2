#pragma once

#include "WorldInstanceData.h"

NS_BEGIN(Engine)

class ENGINE_DLL WorldSerializer : public ISystem
{
public:
	explicit WorldSerializer(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;
	// 1. 현재 월드에서 Export 대상 Entity 수집 저장
	bool SaveWorldToFile(const filesystem::path& outPath, const vector<EntityID>& entityList, string& outErrorMsg);

	// 2. 파일을 읽어 Data 목록을 만들고 스폰 수행
	bool LoadWorldFromFile(const filesystem::path& inPath, vector<EntityID>& outSpawnedEntities, string& outErrorMsg);

	// 3. DTO 레벨 API: 파일 ~DTO, DTO엔티티 분리
	bool SaveDtoToFile(const filesystem::path& outPath, const vector<WorldInstanceDto>& instances, string& outErrorMsg);
	bool LoadDtoFromFile(const filesystem::path& inPath, vector<WorldInstanceDto>& outInstances, string& outErrorMsg);

private:
	// Entity -> DTO 변환 
	bool BuildDtoFromEntity(EntityID entityId, WorldInstanceDto& outDto);
	// DTO -> Entity Spanw
	EntityID SpawnFromDto(const WorldInstanceDto& dto);

private:
	SystemRegistry&  registry;
	EntityMgr*       entities{};
	TransformSystem* tfSys{};
	LayerSystem*     layerSys{};
	ModelSystem*     modelSys{};
	AssetSystem*     assets{};
	EntitySpawner*   spawner{};
	CollisionSystem* collisionSys{};
};

NS_END