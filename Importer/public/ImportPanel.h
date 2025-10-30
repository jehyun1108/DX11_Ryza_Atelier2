#pragma once

NS_BEGIN(Importer)

enum class OverWritePolicy
{
	SkipExisting,   // 기존 .model 있으면 건너뜀
	OverwriteAll    // 기존 .model 있어도 덮어씀
};

class ImportPanel final : public GuiPanel
{
public:
	ImportPanel(string title, SystemRegistry& registry, EntityMgr& entities, EntityID* selected)
		: GuiPanel(move(title), registry, entities, selected), assets(registry.Get<AssetSystem>()){ previewEntities.reserve(512); }

public:
	void Draw() override;

private:
	void RefreshModels();
	void SpawnEntity(const filesystem::path& modelPath);
	void DestroyAll();

	void Save();
	void Load(bool clearBeforeLoad);

	bool IsFBXFile(const filesystem::path& filePath);
	void ImportAll(const filesystem::path& rootFolder, OverWritePolicy overWritePolicy, bool useRecursive);

private:
	filesystem::path baseModelPath;
	vector<filesystem::path> modelFiles;

	vector<EntityID> previewEntities;
	_uint previewCounter = 0;

	AssetSystem& assets;

	// UI 상태 메시지
	string statusMsg;
	bool   clearBeforeLoad = false;
};

NS_END