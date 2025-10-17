#include "pch.h"
#include "ImportPanel.h"

#include "ModelImporter.h"
#include "ModelExporter.h"
#include "WorldSerializer.h"

constexpr wchar_t worldFilter[] = L"World Files (*.dat)\0*.dat\0All Files (*.*)\0*.*\0";

// -fbxmultitake : Animation Channel º°·Î »Ì°Ô 
void ImportPanel::Draw()
{
#ifdef USE_IMGUI
	if (ImGui::Button("Import Model", ImVec2(-1, 0)))
	{
		if (auto selectedFile = Utility::OpenFbxFileDialog())
		{
			baseModelPath = *selectedFile;

			ModelImporter importer;
			auto imported = importer.Import(baseModelPath);

			if (imported)
			{
				ModelExporter exporter;
				auto outPath = baseModelPath;
				outPath.replace_extension(".model");

				if (exporter.Export(*imported, outPath))
					RefreshModels();
			}
		}
	}

	ImGui::Separator();
	ImGui::TextUnformatted("Batch Import (FBX -> .model)");

	static bool overwriteExisting = false;
	static bool useRecursive = true;

	ImGui::Checkbox("Overwrite Existing", &overwriteExisting);
	ImGui::SameLine();
	ImGui::Checkbox("Recursive", &useRecursive);

	if (ImGui::Button("Batch Import from ../bin/Resources/Models/Central/", ImVec2(-1, 0)))
	{
		filesystem::path rootFolder = filesystem::path("..\\bin\\Resources\\Models\\Central\\");
		rootFolder = filesystem::absolute(rootFolder);

		ImportAll(rootFolder, overwriteExisting, useRecursive);
	}

	ImGui::Separator();
	ImGui::TextUnformatted("World I/O");
	ImGui::Checkbox("Clear before Load", &clearBeforeLoad);

	if (ImGui::Button("Save"))
		Save();

	ImGui::SameLine();

	if (ImGui::Button("Load"))
		Load(clearBeforeLoad);

	if (!statusMsg.empty())
	{
		ImGui::Spacing();
		ImGui::TextWrapped("%s", statusMsg.c_str());
	}
	
	ImGui::Separator();
	ImGui::TextUnformatted("Exported Models");
	if (ImGui::Button("Refresh List"))   RefreshModels();
	if (ImGui::Button("Clear Entities")) DestroyAll();

	if (ImGui::BeginListBox("##ModelList", ImVec2(-1, -1)))
	{
		for (const auto& modelPath : modelFiles)
		{
			const string displayName = modelPath.filename().string();
			if (ImGui::Selectable(displayName.c_str()))
				SpawnEntity(modelPath);
		}
		ImGui::EndListBox();
	}

	// -----------------------------------
	ImGuiIO& io = ImGui::GetIO();
	const bool wantUI = io.WantCaptureMouse;
	
	if (!wantUI && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		D3D11_VIEWPORT vp = game.GetViewport();
	
		const _float2 screenPos = { io.MousePos.x, io.MousePos.y };
	
		auto& camSys = registry.Get<CameraSystem>();
		Handle cam = camSys.GetMainCamHandle();
		if (cam.IsValid())
		{
			PickingRequest request{};
			request.fromScreen = true;
			request.screenpos = screenPos;
			request.viewport = vp;
			request.cam = cam;
			request.layerMask = LayerUtil::LayerBit(LAYER::MAPOBJ);
	
			PickingHit hit{};
			auto& pickingSys = registry.Get<PickingSystem>();
			if (pickingSys.PickFromScreen(request, hit) && hit.hit)
			{
				if (selected)
					*selected = hit.entity;
			}
		}
	}
#endif
}

void ImportPanel::RefreshModels()
{
#ifdef USE_IMGUI
	modelFiles.clear();

	filesystem::path relative = PathMgr::GetModelPath();
	filesystem::path absolute = filesystem::absolute(relative);

	for (const auto& entry : filesystem::recursive_directory_iterator(absolute))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".model")
			modelFiles.push_back(entry.path());
	}
	sort(modelFiles.begin(), modelFiles.end());
#endif
}


void ImportPanel::SpawnEntity(const filesystem::path& modelPath)
{
	const wstring& logicalKey = Utility::MakeModelKey(modelPath);
	assets.RegisterModel(logicalKey, { modelPath.wstring(), true });

	EntitySpawner spawner{ registry, entities };

	const size_t spawnedCount = previewEntities.size();
	const int gridX = static_cast<int>(spawnedCount % 10);
	const int gridY = static_cast<int>(spawnedCount / 10);
	const float spacing = 2.f;
	const _float3 spawnPos = _float3(gridX * spacing, 0.f, gridY * spacing);

	auto handles = spawner.NewEntity()
		.WithTf(TransformDesc{ .pos = spawnPos })
		.WithLayer(LayerUtil::LayerBit(LAYER::MAPOBJ))
		.WithModel(logicalKey)
		.WithPickingFromModel()
		.WithColliderFromModel(ColliderType::AABB)
		.Build();

	previewEntities.push_back(handles.entity);
	++previewCounter;
	if (selected)
		*selected = handles.entity;
}

void ImportPanel::DestroyAll()
{
	for (EntityID id : previewEntities)
	{
		if (entities.IsAlive(id))
			entities.DestroyDeferred(id);
	}
	previewEntities.clear();
}

void ImportPanel::Save()
{
	statusMsg.clear();

	const wstring filter = worldFilter;

	auto maybeOut = Utility::SaveFileDialog(filter, L"map.dat", L"dat");
	if (!maybeOut.has_value())
	{
		statusMsg = "Save canceled";
		return;
	}

	vector<EntityID> toSave;
	toSave.reserve(previewEntities.size());
	for (EntityID id : previewEntities)
	{
		if (entities.IsAlive(id))
			toSave.push_back(id);
	}

	if (toSave.empty())
	{
		statusMsg = "No preview entities to save";
		return;
	}

	WorldSerializer serializer(registry, entities);
	string errorMsg;
	if (serializer.SaveWorldToFile(*maybeOut, toSave, errorMsg))
		statusMsg = string("Saved: ") + maybeOut->string();
	else
		statusMsg = string("Saved failed: ") + errorMsg;
}

void ImportPanel::Load(bool clearBeforeLoad)
{
	statusMsg.clear();

	const wstring filter = worldFilter;

	auto maybeIn = Utility::OpenFileDialog(filter, L"dat");
	if (!maybeIn)
	{
		statusMsg = "Load canceled";
		return;
	}

	if (clearBeforeLoad)
		DestroyAll();

	WorldSerializer  serializer(registry, entities);
	vector<EntityID> spawned;
	string errorMsg;
	if (serializer.LoadWorldFromFile(*maybeIn, spawned, errorMsg))
	{
		previewEntities.insert(previewEntities.end(), spawned.begin(), spawned.end());
		if (!spawned.empty() && selected)
			*selected = spawned.back();

		statusMsg = string("Loaded: ") + maybeIn->string() + " | spawned " + to_string(spawned.size()) + " entities.";
	}
	else
		statusMsg = string("Load failed: ") + errorMsg;
}

bool ImportPanel::IsFBXFile(const filesystem::path& filePath)
{
	const wstring ext = Utility::ToLower(filePath.extension().wstring());
	return (ext == L".fbx");
}

void ImportPanel::ImportAll(const filesystem::path& rootFolder, bool overwriteExisting, bool useRecursive)
{
	statusMsg.clear();

	if (!filesystem::exists(rootFolder) || !filesystem::is_directory(rootFolder))
	{
		statusMsg = string("Invalid folder: ") + rootFolder.string();
		return;
	}

	size_t totalFoundCount = 0;
	size_t convertedCount  = 0;
	size_t skippedCount    = 0;
	size_t failedCount     = 0;

	auto ProcessOneFbx = [&](const filesystem::path& fbxPath)
		{
			++totalFoundCount;

			filesystem::path outPath = fbxPath;
			outPath.replace_extension(L".model");

			const bool modelExists = filesystem::exists(outPath);
			if (modelExists && !overwriteExisting)
			{
				++skippedCount;
				return;
			}

			ModelImporter importer;
			auto importedModel = importer.Import(fbxPath);
			if (!importedModel)
			{
				++failedCount;
				return;
			}

			ModelExporter exporter;
			if (exporter.Export(*importedModel, outPath))
				++convertedCount;
			else
				++failedCount;
		};

	if (useRecursive)
	{
		for (const auto& entry : filesystem::recursive_directory_iterator(rootFolder))
		{
			if (entry.is_regular_file() && IsFBXFile(entry.path()))
				ProcessOneFbx(entry.path());
		}
	}

	RefreshModels();
	statusMsg = "Batch Import Finished.  Found: " + to_string(totalFoundCount) + ", Converted: " + to_string(convertedCount) + ", Skipped: " + to_string(skippedCount) + ", Failed: " + to_string(failedCount);
}
