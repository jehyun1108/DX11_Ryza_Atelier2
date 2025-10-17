#pragma once

NS_BEGIN(Importer)

class ImportPanel final : public GuiPanel
{
public:
	ImportPanel(string title, SystemRegistry& registry, EntityMgr& entities, EntityID* selected)
		: GuiPanel(move(title), registry, entities, selected), assets(registry.Get<AssetSystem>())
	{
		previewEntities.reserve(256);
	}

public:
	void Draw() override;

private:
	void RefreshModels();
	void SpawnEntity(const filesystem::path& modelPath);
	void DestroyAll();

	void Save();
	void Load(bool clearBeforeLoad);

	bool IsFBXFile(const filesystem::path& filePath);
	void ImportAll(const filesystem::path& rootFolder, bool overwriteExisting, bool useRecursive);

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