#include "pch.h"

void MapToolPanel::Draw()
{
#ifdef USE_IMGUI

	RefreshModelList();

	ImGui::Text("Tools");
	if (ImGui::RadioButton("Select", curState == MAPTOOL::SELECT))
	{
		curState = MAPTOOL::SELECT;
		if (previewObj)
		{
			game.DestroyObj(previewObj);
			previewObj = nullptr;
		}
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Place", curState == MAPTOOL::PLACE))
		curState = MAPTOOL::PLACE;
	ImGui::Separator();
	// --------------------------------------------------------------
	ImGui::BeginDisabled(curState != MAPTOOL::PLACE);
	if (ImGui::BeginListBox("##ModelList", ImVec2(-1.f, -1.f)))
	{
		for (int i = 0; i < modelFiles.size(); ++i)
		{
			const auto& path = modelFiles[i];
			const string modelName = path.stem().string();
			const bool isSelected = (selectedModelIdx == i);

			if (ImGui::Selectable(modelName.c_str(), isSelected))
			{
				selectedModelIdx = i;
				game.LoadModel(path.wstring());
				Utility::Log(L"Selected model for preview: {}", path.stem().wstring());

				if (previewObj)
				{
					game.DestroyObj(previewObj);
					previewObj = nullptr;
				}

				ObjDesc desc;
				desc.levelID = ENUM(LEVEL::MAPTOOL);
				desc.modelKey = path.stem().wstring();
				desc.layerType = LAYER::MAPOBJ;
				desc.name = L"_PreviewObj_";
				previewObj = game.AddObj<MapObj>(desc);
			}

			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndListBox();
	}
	ImGui::EndDisabled();

#endif
}

void MapToolPanel::Update(float dt)
{
#ifdef USE_IMGUI

	UpdateTool();

#endif
}

void MapToolPanel::UpdateTool()
{
#ifdef USE_IMGUI
	if (ImGui::GetIO().WantCaptureMouse) return;

	auto mainCam = game.GetMainCam();
	if (!mainCam) return;

	if (curState == MAPTOOL::PLACE && previewObj)
	{
		_vec rayOrigin, rayDir;
		mainCam->CreateRayFromScreen(game.GetMousePos(), rayOrigin, rayDir);

		if (grid)
		{
			GridCoord  cell{};
			_float3    hit{};
			const bool inside = grid->PickCell(rayOrigin, rayDir, cell, hit);

			if (inside)
			{
				_float3 center = grid->CellToWorldCenter(cell);
				const auto& gridParam = grid->GetParams();
				center.y = gridParam.origin.y + placeYOffset;
				previewObj->GetComp<Transform>()->SetPos(XMLoadFloat3(&center));

				float minX, maxX, minZ, maxZ;
				previewObj->GetWorldXZFootprint(minX, maxX, minZ, maxZ);

				_float3 minW{ minX, gridParam.origin.y, minZ };
				_float3 maxW{ maxX, gridParam.origin.y, maxZ };

				grid->SetHoverForAABB(minW, maxW, true);
			}
			else
				grid->SetHoverArea(CellRect{}, false);
		}
		else
		{
			const float planeY = 0.f;
			_vec plane = XMVectorSet(0.f, 1.f, 0.f, -planeY);
			_vec hit4 = XMPlaneIntersectLine(plane, rayOrigin, rayOrigin + rayDir * 10000.f);
			if (!XMVector3IsInfinite(hit4) && !XMVector3IsNaN(hit4))
				previewObj->GetComp<Transform>()->SetPos(hit4);
		}
	}

	if (game.KeyDown(KEY::LBUTTON))
	{
		_vec rayOrigin, rayDir;
		mainCam->CreateRayFromScreen(game.GetMousePos(), rayOrigin, rayDir);

		switch (curState)
		{
		case MAPTOOL::SELECT:
		{
			auto& objs = game.GetActiveObjs();
			float closestDist = FLT_MAX;
			pickedObj = nullptr;

			for (const auto& obj : objs)
			{
				MapObj* mapObj = dynamic_cast<MapObj*>(obj);
				if (!mapObj || mapObj == previewObj)
					continue;

				float dist = 0.f;
				if (mapObj->GetWorldBoundingBox().Intersects(rayOrigin, rayDir, dist))
				{
					if (dist < closestDist)
					{
						closestDist = dist;
						pickedObj = mapObj;
					}
				}
			}
		}
		break;

		case MAPTOOL::PLACE:
		{
			if (previewObj && grid)
			{
				float minX, maxX, minZ, maxZ;
				previewObj->GetWorldXZFootprint(minX, maxX, minZ, maxZ);

				const auto& gridParam = grid->GetParams();
				_float3 minW{ minX, gridParam.origin.y, minZ };
				_float3 maxW{ maxX, gridParam.origin.y, maxZ };

				const bool fullyInside = grid->SetHoverForAABB(minW, maxW, true);

				if (fullyInside)
				{
					previewObj->SetName(previewObj->GetName() + L"_" + to_wstring(objCounter++));
					previewObj = nullptr;

					grid->SetHoverArea(CellRect{}, false);
				}
			}
		}
		break;

		}
	}
#endif
}

optional<filesystem::path> MapToolPanel::GetSelectedModelPath() const
{
#ifdef USE_IMGUI
	if (selectedModelIdx >= 0 && selectedModelIdx < modelFiles.size())
		return modelFiles[selectedModelIdx];

	return nullopt;

#endif
}

void MapToolPanel::RefreshModelList()
{
#ifdef USE_IMGUI
	modelFiles.clear();
	selectedModelIdx = -1;

	filesystem::path modelPath = L"../bin/Resources/MapObjs/";

	for (const auto& entry : filesystem::recursive_directory_iterator(modelPath))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".model")
			modelFiles.push_back(entry.path());
	}
#endif
}
