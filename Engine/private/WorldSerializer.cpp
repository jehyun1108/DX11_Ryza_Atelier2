#include "Enginepch.h"
#include "WorldSerializer.h"

static constexpr _uint worldMagic   = 'W' | ('L' << 8) | ('D' << 16) | ('1' << 24); // "WLD1"
static constexpr _uint worldVersion = 1;

bool WorldSerializer::SaveWorldToFile(const filesystem::path& outPath, const vector<EntityID>& entityList, string& outErrorMsg)
{
	if (entityList.empty())
	{
		outErrorMsg = "No entities to save";
		return false;
	}

	vector<WorldInstanceDto> dtoList;
	dtoList.reserve(entityList.size());

	for (EntityID entityId : entityList)
	{
		if (!entities.IsAlive(entityId)) continue;

		WorldInstanceDto dto{};
		if (!BuildDtoFromEntity(entityId, dto))
		{
			outErrorMsg = "Failed to build DTO for entity";
			return false;
		}
		dtoList.push_back(dto);
	}

	if (dtoList.empty())
	{
		outErrorMsg = "No valid entities to save.";
		return false;
	}
	return SaveDtoToFile(outPath, dtoList, outErrorMsg);
}

bool WorldSerializer::LoadWorldFromFile(const filesystem::path& inPath, vector<EntityID>& outSpawnedEntities, string& outErrorMsg)
{
	vector<WorldInstanceDto> dtoList;
	if (!LoadDtoFromFile(inPath, dtoList, outErrorMsg))
		return false;

	outSpawnedEntities.clear();
	outSpawnedEntities.reserve(dtoList.size());

	for (const WorldInstanceDto& dto : dtoList)
	{
		EntityID spawned = SpawnFromDto(dto);
		if (spawned != invalidEntity)
			outSpawnedEntities.push_back(spawned);
	}
	return true;
}

bool WorldSerializer::SaveDtoToFile(const filesystem::path& outPath, const vector<WorldInstanceDto>& instances, string& outErrorMsg)
{
	ofstream outFile = BinaryUtil::OpenOut(outPath);
	if (!outFile)
	{
		outErrorMsg = "Failed to open file: " + outPath.string();
		return false;
	}
	BinaryUtil::WriteHeader(outFile, worldMagic, worldVersion);

	const _uint count = static_cast<_uint>(instances.size());
	BinaryUtil::WritePOD(outFile, count);

	for (const WorldInstanceDto& instance : instances)
	{
		// UTF-8
		BinaryUtil::WriteString(outFile, instance.logicalKey);

		// Pos
		BinaryUtil::WritePOD(outFile, instance.pos[0]);
		BinaryUtil::WritePOD(outFile, instance.pos[1]);
		BinaryUtil::WritePOD(outFile, instance.pos[2]);

		// Rot
		BinaryUtil::WritePOD(outFile, instance.rotQuat[0]);
		BinaryUtil::WritePOD(outFile, instance.rotQuat[1]);
		BinaryUtil::WritePOD(outFile, instance.rotQuat[2]);
		BinaryUtil::WritePOD(outFile, instance.rotQuat[3]);

		// Scale
		BinaryUtil::WritePOD(outFile, instance.scale[0]);
		BinaryUtil::WritePOD(outFile, instance.scale[1]);
		BinaryUtil::WritePOD(outFile, instance.scale[2]);

		// LayerMask
		BinaryUtil::WritePOD(outFile, instance.layerMask);
	}
	return true;
}

bool WorldSerializer::LoadDtoFromFile(const filesystem::path& inPath, vector<WorldInstanceDto>& outInstances, string& outErrorMsg)
{
	ifstream inFile = BinaryUtil::OpenIn(inPath);
	if (!inFile)
	{
		outErrorMsg = "Failed to open file: " + inPath.string();
		return false;
	}

	_uint version{};
	if (!BinaryUtil::ReadHeader(inFile, worldMagic, version))
	{
		outErrorMsg = "Invalid world header";
		return false;
	}
	if (version != worldVersion)
	{
		outErrorMsg = "Unsupported world version";
		return false;
	}

	_uint count = 0;
	if (!BinaryUtil::ReadPOD(inFile, count))
	{
		outErrorMsg = "Failed to read instance count";
		return false;
	}

	outInstances.clear();
	outInstances.reserve(count);

	for (_uint i = 0; i < count; ++i)
	{
		WorldInstanceDto instance{};

		if (!BinaryUtil::ReadString(inFile, instance.logicalKey))
		{
			outErrorMsg = "Failed ot read logicalKey";
			return false;
		}

		if (!BinaryUtil::ReadPOD(inFile, instance.pos[0]))   return false;
		if (!BinaryUtil::ReadPOD(inFile, instance.pos[1]))   return false;
		if (!BinaryUtil::ReadPOD(inFile, instance.pos[2]))   return false;

		if (!BinaryUtil::ReadPOD(inFile, instance.rotQuat[0])) return false;
		if (!BinaryUtil::ReadPOD(inFile, instance.rotQuat[1])) return false;
		if (!BinaryUtil::ReadPOD(inFile, instance.rotQuat[2])) return false;
		if (!BinaryUtil::ReadPOD(inFile, instance.rotQuat[3])) return false;

		if (!BinaryUtil::ReadPOD(inFile, instance.scale[0])) return false;
		if (!BinaryUtil::ReadPOD(inFile, instance.scale[1])) return false;
		if (!BinaryUtil::ReadPOD(inFile, instance.scale[2])) return false;

		if (!BinaryUtil::ReadPOD(inFile, instance.layerMask)) return false;

		outInstances.push_back(instance);
	}
	return true;
}

bool WorldSerializer::BuildDtoFromEntity(EntityID entityId, WorldInstanceDto& outDto)
{
	// 1. Transform 
	Handle tfHandle{};
	const TransformData* tf = tfSys.GetByOwner(entityId, &tfHandle);
	if (!tf) return false;

	const _float3 pos = tf->pos;
	const _float4 rot = tf->rot;
	const _float3 scale = tf->scale;

	outDto.pos[0] = pos.x;
	outDto.pos[1] = pos.y;
	outDto.pos[2] = pos.z;

	// 쿼터니언: x,y,z,w 로 저장 + 정규화
	{
		float quatX = tf->rot.x;
		float quatY = tf->rot.y;
		float quatZ = tf->rot.z;
		float quatW = tf->rot.w;

		const float quatLength = quatX * quatX + quatY * quatY + quatZ * quatZ + quatW * quatW;
		if (quatLength > 1e-12f)
		{
			const float invLength = 1.f / sqrtf(quatLength);
			quatX *= invLength;
			quatY *= invLength;
			quatZ *= invLength;
		}
		else
		{
			quatX = 0.f;
			quatY = 0.f;
			quatZ = 0.f;
			quatW = 1.f;
		}

		outDto.rotQuat[0] = quatX;
		outDto.rotQuat[1] = quatY;
		outDto.rotQuat[2] = quatZ;
		outDto.rotQuat[3] = quatW;
	}

	{
		const float scaleX = (fabsf(tf->scale.x) < 1e-6f) ? 1e-6f : tf->scale.x;
		const float scaleY = (fabsf(tf->scale.y) < 1e-6f) ? 1e-6f : tf->scale.y;
		const float scaleZ = (fabsf(tf->scale.z) < 1e-6f) ? 1e-6f : tf->scale.z;

		outDto.scale[0] = scaleX;
		outDto.scale[1] = scaleY;
		outDto.scale[2] = scaleZ;
	}

	// 2. Layer
	const LayerData* layer = layerSys.GetByOwner(entityId);
	outDto.layerMask = layer ? layer->layerMask : 0xFFFFFFFFu;

	// 3. Model -> LogicalKey 얻어오기
	const ModelData* model = modelSys.GetByOwner(entityId);
	if (!model || !model->model) return false;

	const wstring logicalKey = model->model->GetLogicalKey();
	if (logicalKey.empty()) return false;

	const wstring normalizedKey = Utility::Normalize(logicalKey);
	outDto.logicalKey           = Utility::ToString(normalizedKey);
		
	return true;
}

EntityID WorldSerializer::SpawnFromDto(const WorldInstanceDto& dto)
{
	// 1. DTO의 UTF-8 LogicalKey -> UTF-16 -> 정규화
	const wstring wKey          = Utility::ToWString(dto.logicalKey);
	const wstring normalizedKey = Utility::Normalize(wKey);

	// 2. Meta & 경로
    filesystem::path fullPath = PathMgr::GetAssetPath() / normalizedKey;
	fullPath.replace_extension(L".model");

	ModelMeta meta{};
	meta.fullPath         = fullPath.wstring();
	meta.resolveMaterials = true;

	assets.RegisterModel(normalizedKey, meta);

	// 3. Spawn
	EntitySpawner spawner{ registry, entities };

	TransformDesc tfDesc{};
	tfDesc.pos   = _float3(dto.pos[0], dto.pos[1], dto.pos[2]);
	
	// Quat
	{
		float quatX = dto.rotQuat[0];
		float quatY = dto.rotQuat[1];
		float quatZ = dto.rotQuat[2];
		float quatW = dto.rotQuat[3];

		const float quatLength = quatX * quatX + quatY * quatY + quatZ * quatZ + quatW * quatW;
		if (quatLength > 1e-12f)
		{
			const float invLength = 1.f / sqrtf(quatLength);
			quatX *= invLength;
			quatY *= invLength;
			quatZ *= invLength;
			quatW *= invLength;
		}
		else
		{
			quatX = 0.f;
			quatY = 0.f;
			quatZ = 0.f;
			quatW = 0.f;
		}

		tfDesc.rot = _float4(quatX, quatY, quatZ, quatW);
	}
	
	{
		float scaleX = (fabsf(dto.scale[0]) < 1e-6f) ? 1e-6f : dto.scale[0];
		float scaleY = (fabsf(dto.scale[1]) < 1e-6f) ? 1e-6f : dto.scale[1];
		float scaleZ = (fabsf(dto.scale[2]) < 1e-6f) ? 1e-6f : dto.scale[2];
		tfDesc.scale = _float3(scaleX, scaleY, scaleZ);
	}

	auto handles = spawner.NewEntity()
		.WithTf(tfDesc)
		.WithLayer(dto.layerMask)
		.WithModel(normalizedKey)
		.WithPickingFromModel()
		.WithColliderFromModel(ColliderType::AABB)
		.Build();

	return handles.entity;
}
