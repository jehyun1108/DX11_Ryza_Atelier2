#include "pch.h"
#include "ImportPanel.h"

#include "ModelImporter.h"
#include "ModelExporter.h"
#include "WorldSerializer.h"
#include "NavMeshSystem.h"
#include "PickingSystem.h"
#include "TransformSystem.h"
#include "InputService.h"
#include "FontImporter.h"

//#include "stb_image_write.h"
//#include "stb_rect_pack.h"
#include "stb_truetype.h"


namespace
{
	bool LoadFileBinary(const filesystem::path& path, vector<unsigned char>& out)
	{
		ifstream ifs(path, ios::binary);
		if (!ifs)
			return false;

		ifs.seekg(0,ios::end);
		streamsize size = ifs.tellg();
		ifs.seekg(0, ios::beg);

		if (size <= 0)
			return false;

		out.resize(static_cast<size_t>(size));
		if (!ifs.read(reinterpret_cast<char*>(out.data()), size))
			return false;

		return true;
	}
}

constexpr wchar_t worldFilter[] = L"World Files (*.dat)\0*.dat\0All Files (*.*)\0*.*\0";

ImportPanel::ImportPanel(string title, SystemRegistry& registry, EntityID* selected)
	:GuiPanel(move(title), registry, selected)
{
	entities   = &registry.Get<EntityMgr>();
	spawner    = &registry.Get<EntitySpawner>();
	serializer = &registry.Get<WorldSerializer>();
	nav        = &registry.Get<NavMeshSystem>();
	pickSys    = &registry.Get<PickingSystem>();
	tfSys      = &registry.Get<TransformSystem>();
	camSys     = &registry.Get<CameraSystem>();
	input      = &registry.Get<InputService>();
	assets     = &registry.Get<AssetSystem>();

	previewEntities.reserve(512);
}

// -fbxmultitake -fbxascii -notex
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
// --------------------------------------------------------------------------------------
	ImGui::Separator();
	ImGui::TextUnformatted("Batch Import (FBX -> .model)");

	static bool useRecursive = true;
	ImGui::Checkbox("Recursive", &useRecursive);

	filesystem::path batchRootFolder = filesystem::absolute( filesystem::path("..\\bin\\Resources\\Models\\Central\\"));

	if (ImGui::Button("Import New Only (Skip Existing)", ImVec2(-1, 0)))
		ImportAll(batchRootFolder, OverWritePolicy::SkipExisting, useRecursive);

	if (ImGui::Button("Reimport All (Overwrite Existing)", ImVec2(-1, 0)))
		ImGui::OpenPopup("Confirm Reimport All");

	if (ImGui::BeginPopupModal("Confirm Reimport All", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextWrapped("This will overwrite existing .model files under:\n%s\n\nProceed?",
			batchRootFolder.string().c_str());
		ImGui::Separator();
		if (ImGui::Button("Yes, Overwrite All", ImVec2(180, 0)))
		{
			ImportAll(batchRootFolder, OverWritePolicy::OverwriteAll, useRecursive);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
	// ======================================================================================
	ImGui::Separator();
	ImGui::TextUnformatted("Font Import (TTF -> Atlas)");

	constexpr wchar_t fontFilter[] =
		L"Font files (*.ttf;*.otf)\0*.ttf;*.otf\0All Files (*.*)\0*.*\0";

	if (ImGui::Button("Import Font (TTF -> Atlas)", ImVec2(-1, 0)))
	{
		auto maybeFont = Utility::OpenFileDialog(fontFilter, L"ttf;otf");
		if (!maybeFont)
			statusMsg = "Font import canceled";
		else
			ImportFontFromTTF(*maybeFont);
	}
	// ------------------- NavMesh -------------------------------------------------------
	constexpr wchar_t navFilter[] = L"NavMesh (*.nav;*.bin)\0*.nav;*.bin\0All Files (*.*)\0*.*\0";
	ImGui::Separator();
	ImGui::TextUnformatted("NavMesh I/O");

	if (ImGui::Button("Save NavMesh", ImVec2(-1, 0)))
	{
		auto out = Utility::SaveFileDialog(navFilter, L"navmesh.nav", L"nav");
		if (!out) statusMsg = "Nav save canceled";
		else      statusMsg = nav->Save(*out) ? ("Nav saved: " + out->string())
			: "Nav save failed";
	}

	if (ImGui::Button("Load NavMesh", ImVec2(-1, 0)))
	{
		auto in = Utility::OpenFileDialog(navFilter, L"nav;bin");
		if (!in) statusMsg = "Nav load canceled";
		else
		{
			statusMsg = nav->Load(*in) ? ("Nav loaded: " + in->string()) : "Nav load failed";
		}
	}
	if (ImGui::Button("Undo Triangle"))
		nav->DeleteLastTriangle();

	if (registry.Get<InputMgr>().KeyDown(KEY::NUM1))
		nav->DeleteLastTriangle();

	ImGui::SameLine();
	if (ImGui::Button("Undo Point"))
		nav->UndoLastPoint();
	// ----------------------------------------------------------------------------------
	ImGui::Separator();
	ImGui::TextUnformatted("World I/O");
	ImGui::Checkbox("Clear before Load", &clearBeforeLoad);

	if (ImGui::Button("Save")) Save();
	ImGui::SameLine();
	if (ImGui::Button("Load")) Load(clearBeforeLoad);

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

	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		ImGuiIO& io = ImGui::GetIO();

		if (io.WantCaptureMouse)
			return;

		D3D11_VIEWPORT vp = game.GetViewport();
		ImGuiViewport* mainVp = ImGui::GetMainViewport();

		float localX = io.MousePos.x - mainVp->Pos.x;
		float localY = io.MousePos.y - mainVp->Pos.y;

		_float2 screenPos = 
		{
			localX * io.DisplayFramebufferScale.x,
			localY * io.DisplayFramebufferScale.y
		};

		Handle cam = camSys->GetMainCamHandle();
		if (cam.IsValid())
		{
			PickingRequest request{};
			request.fromScreen = true;
			request.screenpos = screenPos;
			request.viewport = vp;
			request.cam = cam;
			request.layerMask = LayerUtil::LayerBit(LAYER::TERRAIN);

			PickingHit hit{};
			if (pickSys->Pick(request, hit) && hit.hit)
				nav->PushPointFromPick(hit.point, hit.normal);
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
	assets->RegisterModel(logicalKey, { modelPath.wstring(), true });

	const size_t spawnedCount = previewEntities.size();
	const int gridX = static_cast<int>(spawnedCount % 10);
	const int gridY = static_cast<int>(spawnedCount / 10);
	const float spacing = 2.f;
	const _float3 spawnPos = _float3(gridX * spacing, 0.f, gridY * spacing);

	auto handles = spawner->NewEntity()
		.WithTf(TransformDesc{ .pos = spawnPos })
		.WithLayer(LayerUtil::LayerBit(LAYER::MAPOBJ))
		.WithModel(logicalKey)
		.WithMeshCollider()
		.WithPickable()
		.WithSelectable()
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
		if (entities->IsAlive(id))
			entities->DestroyDeferred(id);
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
		if (entities->IsAlive(id))
			toSave.push_back(id);
	}

	if (toSave.empty())
	{
		statusMsg = "No preview entities to save";
		return;
	}

	string errorMsg;
	if (serializer->SaveWorldToFile(*maybeOut, toSave, errorMsg))
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

	vector<EntityID> spawned;
	string errorMsg;
	if (serializer->LoadWorldFromFile(*maybeIn, spawned, errorMsg))
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

void ImportPanel::ImportAll(const filesystem::path& rootFolder, OverWritePolicy overWritePolicy, bool useRecursive)
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

			filesystem::path outputModelPath = fbxPath;
			outputModelPath.replace_extension(L".model");

			const bool modelAlreadyExists = filesystem::exists(outputModelPath);
			const bool shouldSkip = (overWritePolicy == OverWritePolicy::SkipExisting) && modelAlreadyExists;

			if (shouldSkip)
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
			if (exporter.Export(*importedModel, outputModelPath))
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
	else
	{
		for (const auto& entry : filesystem::directory_iterator(rootFolder))
		{
			if (entry.is_regular_file() && IsFBXFile(entry.path()))
				ProcessOneFbx(entry.path());
		}
	}

	RefreshModels();
	statusMsg = "Batch Import Finished.  Found: " + to_string(totalFoundCount)
		+ ", Converted: " + to_string(convertedCount)
		+ ", Skipped: " + to_string(skippedCount)
		+ ", Failed: " + to_string(failedCount);
}

void ImportPanel::ImportFontFromTTF(const filesystem::path& fontPath)
{
	FontBuildConfig cfg{};
	cfg.ttfPath        = fontPath;
	cfg.pixelHeight    = 48.f;
	cfg.firstCodepoint = 0x0020;
	cfg.lastCodepoint  = 0xD7A3;
	cfg.atlasWidth     = 8192;
	cfg.atlasHeight    = 8192;

	FontAtlasResult result{};
	std::string error;

	if (!BuildFontAtlasFromTTF(cfg, result, error))
	{
		statusMsg = error;
		return;
	}

	// PNG 경로
	filesystem::path baseName = fontPath.stem();        // 예: "consola"
	std::wstring sizeTag = L"_48";                      // pixelHeight 기준으로 나중에 바꿔도 됨

	filesystem::path pngPath = fontPath.parent_path() /
		(baseName.wstring() + sizeTag + L".png");
	filesystem::path fontPathBin = fontPath.parent_path() /
		(baseName.wstring() + sizeTag + L".font");

	if (!SaveFontAtlasPNG(pngPath, result, error))
	{
		statusMsg = error;
		return;
	}

	if (!SaveFontMetaBinary(fontPathBin, result, error))
	{
		statusMsg = error;
		return;
	}

	// 디버그 로그용으로 A 글자 한 번 찍기
	auto itA = result.font.glyphs.find('A');
	if (itA != result.font.glyphs.end())
	{
		const Glyph& g = itA->second;
		statusMsg = "Atlas + font saved. glyph 'A': adv=" +
			std::to_string(g.metrics.advance) +
			" w=" + std::to_string(g.metrics.width) +
			" h=" + std::to_string(g.metrics.height) +
			" -> " + pngPath.string() +
			" / " + fontPathBin.string();
	}
	else
	{
		statusMsg = "Atlas + font saved (but 'A' not in range?)";
	}
}