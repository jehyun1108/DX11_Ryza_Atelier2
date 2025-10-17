#include "Enginepch.h"

Handle PickingSystem::Create(EntityID owner, Handle transform, const BoundingBox& localBox, _uint layerMask, bool enabled)
{
	Handle handle = CreateComp(owner);
	if (auto comp = Get(handle))
	{
		comp->transform = transform;
		comp->localBox  = localBox;
		comp->layerMask = layerMask;
		comp->enabled   = enabled;
		comp->worldBox  = localBox;
		RebuildWorldAABB(handle);
	}
	return handle;
}

void PickingSystem::SetLocalBox(Handle handle, const BoundingBox& localBox)
{
	if (auto comp = Get(handle))
	{
		comp->localBox = localBox;
		RebuildWorldAABB(handle);
	}
}

void PickingSystem::SetEnabled(Handle handle, bool enabled)
{
	if (auto comp = Get(handle))
		comp->enabled = enabled;
}

void PickingSystem::SetLayerMask(Handle handle, _uint mask)
{
	if (auto comp = Get(handle))
		comp->layerMask = mask;
}

void PickingSystem::RebuildWorldAABB(Handle handle)
{
	auto comp = Get(handle);
	if (!comp) return;

	auto& tfSys = registry.Get<TransformSystem>();
	const TransformData* tf = tfSys.Get(comp->transform);
	if (!tf)
	{
		comp->worldBox = comp->localBox;
		return;
	}

	const _mat world = tf->dirty ? Utility::MakeWorldMat(*tf) : XMLoadFloat4x4(&tf->world);
	comp->localBox.Transform(comp->worldBox, world);
	XMStoreFloat4x4(&comp->cacheWorld, world);
}

void PickingSystem::Update(float dt)
{
	auto& tfSys = registry.Get<TransformSystem>();

	ForEachAliveEx([&](Handle handle, EntityID, PickingData& comp)
		{
			if (!comp.enabled) return;

			const TransformData* tf = tfSys.Get(comp.transform);
			if (!tf)
			{
				comp.worldBox = comp.localBox;
				return;
			}

			bool isdirty = false;
			if (tf->dirty) 
				isdirty = true;
			else if (memcmp(&comp.cacheWorld, &tf->world, sizeof(_float4x4)) != 0)
					isdirty = true;

			if (isdirty)
			{
				const _mat world = tf->dirty ? Utility::MakeWorldMat(*tf) : XMLoadFloat4x4(&tf->world);
				comp.localBox.Transform(comp.worldBox, world);
				comp.cacheWorld = tf->world;
			}
		});
}

void PickingSystem::RebuildAll(Handle handle, _uint mask)
{
	ForEachAliveEx([&](Handle handle, EntityID, PickingData&){ RebuildWorldAABB(handle);});
}

bool PickingSystem::RaycastSingle(Handle handle, const _float3& origin, const _float3& dir, float& outDist) const
{
	const auto comp = Get(handle);
	if (!comp || !comp->enabled) return false;

	const _vec vOrigin = XMLoadFloat3(&origin);
	const _vec vDir = XMVector3Normalize(XMLoadFloat3(&dir)); 

	float t = 0.f;
	if (comp->worldBox.Intersects(vOrigin, vDir, t))
	{
		outDist = t;
		return true;
	}
	return false;
}

bool PickingSystem::RaycastAll(const _float3& origin, const _float3& dir, _uint layerMask, PickingHit& outNearest)
{
	auto doRaycast = [&](const _vec& vOrigin, const _vec& vDir, _uint mask, PickingHit& outHit) -> bool
		{
			outHit = {};
			outHit.hit = false;
			outHit.distance = FLT_MAX;

			bool anyHit = false;

			ForEachAliveEx([&](Handle handle, EntityID owner, const PickingData& comp)
				{
					if (!comp.enabled) return;
					if ((comp.layerMask & layerMask) == 0) return;

					float t = 0.f;
					if (comp.worldBox.Intersects(vOrigin, vDir, t))
					{
						if (t >= 0.f && t < outNearest.distance)
						{
							anyHit = true;
							outHit.hit = true;
							outHit.distance = t;
							outHit.entity = owner;

							const _vec point = XMVectorMultiplyAdd(vDir, XMVectorReplicate(t), vOrigin);
							XMStoreFloat3(&outHit.point, point);
						}
					}
				});
			return anyHit;
		};

	const _vec vOrigin = XMLoadFloat3(&origin);
	const _vec vDir = XMVector3Normalize(XMLoadFloat3(&dir));

	return doRaycast(vOrigin, vDir, layerMask, outNearest);
}

bool PickingSystem::PickFromScreen(const PickingRequest& request, PickingHit& out) const
{
	auto doRaycast = [&](const _vec& vOrigin, const _vec& vDir, _uint mask, PickingHit& outHit) -> bool
		{
			outHit = {};
			outHit.hit = false;
			outHit.distance = FLT_MAX;

			bool anyHit = false;

			ForEachAliveEx([&](Handle handle, EntityID owner, const PickingData& comp)
				{
					if (!comp.enabled) return;
					if ((comp.layerMask & mask) == 0) return;

					float t = 0.f;
					if (comp.worldBox.Intersects(vOrigin, vDir, t))
					{
						if (t >= 0.f && t < outHit.distance)
						{
							anyHit = true;
							outHit.hit = true;
							outHit.distance = t;
							outHit.entity = owner;

							const _vec point = XMVectorMultiplyAdd(vDir, XMVectorReplicate(t), vOrigin);
							XMStoreFloat3(&outHit.point, point);
						}
					}
				});
			return anyHit;
		};

	_float3 origin{}, dir{};
	if (request.fromScreen)
	{
		auto& camSys = registry.Get<CameraSystem>();
		if (!camSys.Validate(request.cam))
			return false;

		_vec vOrigin{}, vDir{};
		camSys.CreateRayFromScreen(request.cam, request.screenpos, request.viewport, vOrigin, vDir);

		XMStoreFloat3(&origin, vOrigin);
		XMStoreFloat3(&dir, XMVector3Normalize(vDir));
	}
	else
	{
		origin = request.rayOrigin;
		const _vec vDir = XMVector3Normalize(XMLoadFloat3(&request.rayDir));
		XMStoreFloat3(&dir, vDir);
	}

	const _vec vOrigin = XMLoadFloat3(&origin);
	const _vec vDir = XMVector3Normalize(XMLoadFloat3(&dir));

	return doRaycast(vOrigin, vDir, request.layerMask, out);
}

void PickingSystem::RenderGui(EntityID id)
{
#ifdef USE_IMGUI
// ------------------------------------------------------------------------------------------
	auto printAABB = [](const char* label, const BoundingBox& box)
		{
			ImGui::Text("%s", label);
			ImGui::Indent();
			ImGui::Text("Center : (%.3f, %.3f, %.3f)", box.Center.x,  box.Center.y,  box.Center.z);
			ImGui::Text("Extents: (%.3f, %.3f, %.3f)", box.Extents.x, box.Extents.y, box.Extents.z);
			ImGui::Unindent();
		};

	int total = 0;
	int enabledCount = 0;

	ForEachOwned(id, [&](Handle handle, PickingData& data)
		{
			++total;
			if (data.enabled)
				++enabledCount;
		});

	if (ImGui::CollapsingHeader("Picking: "))
	{
		ImGui::Text("Components: %d (enabled: %d)", total, enabledCount);
		ImGui::Separator();

		ForEachOwned(id, [&](Handle handle, PickingData& comp)
			{
				ImGui::PushID((int)handle.idx);

				const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen;

				if (ImGui::TreeNodeEx("Entry", flags))
				{
					bool enabled = comp.enabled;
					if (ImGui::Checkbox("Enabled", &enabled))
						SetEnabled(handle, enabled);

					// Layer mask
					{
						ImGui::SameLine();
						ImGui::TextDisabled("|");
						ImGui::SameLine();

						_uint layerMask = comp.layerMask;
						if (ImGui::InputScalar("LayerMask", ImGuiDataType_U32, &layerMask, nullptr, nullptr, "%08X", ImGuiInputTextFlags_CharsHexadecimal))
							SetLayerMask(handle, layerMask);
					}

					// Transform
					{
						auto& tfSys = registry.Get<TransformSystem>();
						const TransformData* tf = tfSys.Get(comp.transform);
						const bool tfValid = (tf != nullptr);

						ImGui::Text("Transform: %s (handle=%u)", tfValid ? "OK" : "NULL", (uint32_t)comp.transform.idx);

						if (tfValid)
						{
							ImGui::SameLine();
							ImGui::TextDisabled("|");
							ImGui::SameLine();
							ImGui::Text("dirty: %s", tf->dirty ? "true" : "false");

							// 캐시 행렬과 현재 월드 행렬 비교(바이트 비교)
							bool cacheDiff = (memcmp(&comp.cacheWorld, &tf->world, sizeof(_float4x4)) != 0);
							ImGui::SameLine();
							ImGui::Text("cacheDiff: %s", cacheDiff ? "true" : "false");
						}
					}

					// AABB
					if (ImGui::BeginTable("AABB", 2, ImGuiTableFlags_SizingStretchProp))
					{
						ImGui::TableNextColumn();
						printAABB("Local AABB", comp.localBox);

						ImGui::TableNextColumn();
						printAABB("World AABB", comp.worldBox);

						ImGui::EndTable();
					}

					// 강제 Rebuild / Local -> World 변환
					if (ImGui::Button("Rebuild World AABB"))
						RebuildWorldAABB(handle);

					ImGui::SameLine();
					if (ImGui::Button("Copy Local->World now"))
					{
						auto& tfSys = registry.Get<TransformSystem>();
						if (const TransformData* tf = tfSys.Get(comp.transform))
						{
							const _mat world = XMLoadFloat4x4(&tf->world);
							comp.localBox.Transform(comp.worldBox, world);
							comp.cacheWorld = tf->world;
						}
					}

					// 최근 Raycast 결과(필드가 있으면 표기)
					ImGui::Separator();
					ImGui::Text("Last Raycast: hit=%s, dist=%.3f", comp.lastHit ? "true" : "false", comp.lastDist);

					ImGui::TreePop();
				}
				ImGui::PopID();
			});
	}
#endif
}