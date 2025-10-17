#include "Enginepch.h"

Handle ModelSystem::Create(EntityID owner, Handle transform, const wstring& modelKey, Handle animator)
{
	Handle handle = CreateComp(owner);
	auto& comp   = *Get(handle);
	comp = {};
	comp.transform = transform;

	auto& assets = GAME.GetAssetSystem();
	comp.model = assets.GetModel(modelKey);
	assert(comp.model && "Model resource not found");
	if (!comp.model) return {};

// ------------- Model scale -> x0.1
	if (transform.IsValid())
	{
		auto& tfSys = registry.Get<TransformSystem>();
		if (auto* tf = tfSys.Get(transform))
		{
			tf->scale.x = 0.1f;
			tf->scale.y = 0.1f;
			tf->scale.z = 0.1f;
		}
	}

	// -----------------------------------
	if (comp.model->IsSkeletalModel())
	{
		if (!animator.IsValid())
		{
			const ClipTable* clips = comp.model->GetClipTable();
			const bool hasClips = (clips && !clips->empty());

			if (hasClips)
			{
				auto& animSys = registry.Get<AnimatorSystem>();
				Skeleton* skeleton = comp.model->GetSkeletonRaw();
				comp.animator = animSys.Create(owner, skeleton, clips, transform);
			}
		}
		else
			comp.animator = animator;
	}
	comp.enabled = true;
	return handle;
}

void ModelSystem::SetEnabled(Handle handle, bool on)
{
	if (auto comp = Get(handle))
		comp->enabled = on;
}

void ModelSystem::RenderGui(EntityID id)
{
#ifdef USE_IMGUI
    ForEachOwned(id, [&](Handle handle, ModelData& model)
        {
			ImGui::PushID((int)handle.idx);

			if (ImGui::CollapsingHeader("Model"))
			{
				// Enabled
				bool enabled = model.enabled;
				if (ImGui::Checkbox("Enabled", &enabled))
					SetEnabled(handle, enabled);

				if (!model.model)
				{
					ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "No model assigned.");
					ImGui::PopID();
					return;
				}

				const bool isSkeletal   = model.model->IsSkeletalModel();
				const auto& parts       = model.model->GetParts();
				const BoundingBox& aabb = model.model->GetBoundingBox();

				// ------------------ Summary ----------------------------------
				{
					ScopedCompact compact(false);
					GuiUtility::BeginPanel("Summary", PanelMode::Lines, 2.f);
					{
						if (isSkeletal)  GuiUtility::Badge("Skeletal", ImVec4(0.30f, 0.60f, 1.00f, 1));
						else             GuiUtility::Badge("Static", ImVec4(0.50f, 0.80f, 0.50f, 1));
						ImGui::SameLine();
						ImGui::Text("Parts: %d", (int)parts.size());
						ImGui::SameLine();
						ImGui::Text("| Materials: %d", (int)model.model->GetMaterials().size());

						if (isSkeletal)
						{
							const auto* clipTable = model.model->GetClipTable();
							const int clipCount = (clipTable ? (int)clipTable->size() : 0);
							ImGui::SameLine();
							ImGui::Text("| Clips: %d", clipCount);
						}
					}
					GuiUtility::EndPanel();
				}

				// ----------------- Bounding --------------------------
				{
					//ScopedCompact compact(false);
					GuiUtility::BeginPanel("Bounding Box", PanelMode::Lines, 2.f);
					{
						ImGui::Text("Center : (%.2f, %.2f, %.2f)", aabb.Center.x, aabb.Center.y, aabb.Center.z);
						ImGui::Text("Extents: (%.2f, %.2f, %.2f)", aabb.Extents.x, aabb.Extents.y, aabb.Extents.z);
					}
					GuiUtility::EndPanel();
				}

				{
					// ------------------ Parts -----------------------------
					GuiUtility::BeginPanel("Parts", PanelMode::Lines, 12.f);
					{
						const ImGuiTableFlags flags =
							ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuter |
							ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame;

						if (ImGui::BeginTable("PartsTable", 7, flags))
						{
							ImGui::TableSetupColumn("Idx", ImGuiTableColumnFlags_WidthFixed, 40.f);
							ImGui::TableSetupColumn("MeshKind");
							ImGui::TableSetupColumn("Usage");
							ImGui::TableSetupColumn("Layout");
							ImGui::TableSetupColumn("Vtx", ImGuiTableColumnFlags_WidthFixed, 70.f);
							ImGui::TableSetupColumn("Idx", ImGuiTableColumnFlags_WidthFixed, 70.f);
							ImGui::TableSetupColumn("Material");
							ImGui::TableHeadersRow();

							for (size_t i = 0; i < parts.size(); ++i)
							{
								ImGui::TableNextRow();

								ImGui::PushID((int)i);

								// Idx
								ImGui::TableSetColumnIndex(0);
								ImGui::Text("%zu", i);

								const auto& part = parts[i];
								const Mesh* mesh = part.mesh.get();
								const auto& material = part.material;

								// MeshKind
								ImGui::TableSetColumnIndex(1);
								if (!mesh)
									ImGui::TextDisabled("-");
								else
								{
									switch (mesh->GetMeshKind())
									{
									case MESH::Static:    GuiUtility::Badge("Static",    ImVec4(0.50f, 0.80f, 0.50f, 1)); break;
									case MESH::Skeletal:  GuiUtility::Badge("Skeletal",  ImVec4(0.30f, 0.60f, 1.00f, 1)); break;
									case MESH::Primitive: GuiUtility::Badge("Primitive", ImVec4(0.85f, 0.65f, 0.30f, 1)); break;
									}
								}

								// Usage
								ImGui::TableSetColumnIndex(2);
								if (!mesh) ImGui::TextDisabled("-");
								else {
									switch (mesh->GetUsage())
									{
									case MESHTYPE::Static:   GuiUtility::Badge("Static", ImVec4(0.50f, 0.80f, 0.50f, 1)); break;
									case MESHTYPE::Animated: GuiUtility::Badge("Anim",   ImVec4(0.30f, 0.60f, 1.00f, 1)); break;
									case MESHTYPE::Driver:   GuiUtility::Badge("Driver", ImVec4(0.90f, 0.30f, 0.30f, 1)); break;
									}
								}

								// Layout
								ImGui::TableSetColumnIndex(3);
								if (!mesh) ImGui::TextDisabled("-");
								else {
									const auto layout = mesh->GetLayoutID();
									ImGui::Text("%s",
										layout == VertexLayoutID::PNUTanSkin ? "PNUTanSkin" :
										layout == VertexLayoutID::PNUTan ? "PNUTan" : "Unknown");
								}

								// vertex
								ImGui::TableSetColumnIndex(4);
								ImGui::Text("%u", mesh ? mesh->GetVtxCount() : 0u);

								// Indice
								ImGui::TableSetColumnIndex(5);
								ImGui::Text("%u", mesh ? mesh->GetIdxCount() : 0u);

								// Material
								ImGui::TableSetColumnIndex(6);
								if (!material) ImGui::TextDisabled("None");
								else
								{
									const wchar_t* shaderKey = material->GetShaderKey().c_str();
									if (shaderKey && *shaderKey)
										ImGui::Text("Shader: %ls", shaderKey);
									else
										ImGui::Text("Material");
								}
								ImGui::PopID();
							}
							ImGui::EndTable();
						}
					}
					GuiUtility::EndPanel();
				}
			}
				ImGui::PopID();
        });
#endif
}
