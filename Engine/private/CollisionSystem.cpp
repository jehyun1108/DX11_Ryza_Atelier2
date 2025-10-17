#include "Enginepch.h"

// ------------------------------- Create ---------------------------------------------------------
Handle CollisionSystem::CreateAABB(EntityID owner, Handle tfHandle, const BoundingBox& localBox, _uint layer)
{
	Handle handle = CreateComp(owner);
	if (auto data = Get(handle))
	{
		data->type       = ColliderType::AABB;
		data->enabled    = true;
		data->layerMask  = layer;
		data->tf         = tfHandle;
	
		data->aabb.local = localBox;
		data->aabb.world = localBox;
	}
	return handle;
}

Handle CollisionSystem::CreateSphere(EntityID owner, Handle tfHandle, const _float3& centerLocal, float radiusLocal, _uint layer)
{
	Handle handle = CreateComp(owner);
	if (auto data = Get(handle))
	{
		data->type = ColliderType::Sphere;
		data->enabled = true;
		data->layerMask = layer;
		data->tf = tfHandle;

		data->sphere.centerLocal = centerLocal;
		data->sphere.radiusLocal = radiusLocal;
		data->sphere.centerWorld = centerLocal;
		data->sphere.radiusWorld = radiusLocal;
	}
	return handle;
}

Handle CollisionSystem::CreateOBB(EntityID owner, Handle tfHandle, const BoundingOrientedBox& localOBB, _uint layer)
{
	Handle handle = CreateComp(owner);
	if (auto data = Get(handle))
	{
		data->type        = ColliderType::OBB;
		data->enabled     = true;
		data->layerMask   = layer;
		data->tf          = tfHandle;

		data->obb.local   = localOBB;
		data->obb.world   = localOBB;
	}					  
	return handle;
}

// ----------------------- Runtime switching --------------------------------------
void CollisionSystem::SetAABB(Handle handle, const BoundingBox& localBox)
{
	if (auto data = Get(handle))
	{
		data->type = ColliderType::AABB;
		data->aabb.local = localBox;
	}
}

void CollisionSystem::SetSphere(Handle handle, const _float3& centerLocal, float radiusLocal)
{
	if (auto data = Get(handle))
	{
		data->type               = ColliderType::Sphere;
		data->sphere.centerLocal = centerLocal;
		data->sphere.radiusLocal = radiusLocal;
	}
}

void CollisionSystem::SetOBB(Handle handle, const BoundingOrientedBox& localOBB)
{
	if (auto data = Get(handle))
	{
		data->type = ColliderType::OBB;
		data->obb.local = localOBB;
	}
}

// ----------------------------- Utility --------------------------------------------
void CollisionSystem::SetEnabled(Handle handle, bool enabled)
{
	if (auto data = Get(handle))
		data->enabled = enabled;
}

void CollisionSystem::SetLayer(Handle handle, _uint mask)
{
	if (auto data = Get(handle))
		data->layerMask = mask;
}

void CollisionSystem::SetTransform(Handle handle, Handle tfHandle)
{
	if (auto data = Get(handle))
		data->tf = tfHandle;
}

void CollisionSystem::Update(float dt)
{
	auto& tfSys = registry.Get<TransformSystem>();

	ForEachAliveEx([&](Handle handle, EntityID owner, CollisionData& data)
		{
			if (!data.enabled) return;

			const TransformData* tf = tfSys.Get(data.tf);
			if (!tf)
			{
				switch (data.type)
				{
				case ColliderType::AABB:
					data.aabb.world = data.aabb.local;
					break;

				case ColliderType::Sphere:
					data.sphere.centerWorld = data.sphere.centerLocal;
					data.sphere.radiusWorld = data.sphere.radiusLocal;
					break;

				case ColliderType::OBB:
					data.obb.world = data.obb.local;
					break;
				}
				return;
			}

			const _mat worldMat = XMLoadFloat4x4(&tf->world);

			switch (data.type)
			{
			case ColliderType::AABB:
				data.aabb.local.Transform(data.aabb.world, worldMat);
				break;

			case ColliderType::Sphere:
			{
				// Center 변환 (평행이동 포함)
                const _vec centerLocal = XMLoadFloat3(&data.sphere.centerLocal);
                const _vec centerWorld = XMVector3TransformCoord(centerLocal, worldMat);
                XMStoreFloat3(&data.sphere.centerWorld, centerWorld);

                // Decompose 로 Scale 추출 (비등방 스케일 대응)
                _vec scale, rot, trans;
                if (XMMatrixDecompose(&scale, &rot, &trans, worldMat))
                {
                    const float scaleX = XMVectorGetX(scale);
                    const float scaleY = XMVectorGetY(scale);
                    const float scaleZ = XMVectorGetZ(scale);
                    const float maxScale = max(scaleX, max(scaleY, scaleZ));
                    data.sphere.radiusWorld = data.sphere.radiusLocal * max(1e-6f, maxScale);
                }
                else
                {
                    // column 벡터 길이로 근사 (혹시모를 대비)
                    const _float4x4& m = tf->world;
                    const _float3 columnX = { m._11, m._21, m._31 };
                    const _float3 columnY = { m._12, m._22, m._32 };
                    const _float3 columnZ = { m._13, m._23, m._33 };
                    const float scaleX = Utility::Length(columnX);
                    const float scaleY = Utility::Length(columnY);
                    const float scaleZ = Utility::Length(columnZ);
                    const float maxScale = max(scaleX, max(scaleY, scaleZ));
                    data.sphere.radiusWorld = data.sphere.radiusLocal * max(1e-6f, maxScale);
                }
			}
            break;

			case ColliderType::OBB:
				data.obb.local.Transform(data.obb.world, worldMat);
				break;
			}
		});
}

void CollisionSystem::ExtractColliderProxies(vector<ColliderProxy>& out) const
{
    if (!debugEnabled) return;

    const_cast<CollisionSystem*>(this)->ForEachAlive([&](_uint id, CollisionData& data)
        {
            if (!data.enabled) return;

            ColliderProxy proxy{};
            switch (data.type)
            {
            case ColliderType::AABB:
                proxy.type = ColliderType::AABB;
                proxy.aabb = data.aabb.world;
                break;

            case ColliderType::OBB:
                proxy.type = ColliderType::OBB;
                proxy.obb = data.obb.world;
                break;

            case ColliderType::Sphere:
                proxy.type = ColliderType::Sphere;
                proxy.sphereCenter = data.sphere.centerWorld;
                proxy.sphereRadius = data.sphere.radiusWorld;
                break;
            }
            out.emplace_back(proxy);
        });
}

void CollisionSystem::RenderGui(EntityID id)
{
#ifdef USE_IMGUI
    ForEachOwned(id, [&](Handle handle, CollisionData& data)
        {
            ImGui::PushID((int)handle.idx);

            if (ImGui::CollapsingHeader("Collision"))
            {
                bool on = IsDebugEnabled();
                if (ImGui::Checkbox("Show Colliders", &on))
                    SetDebugEnabled(on);

                GuiUtility::BeginPanel("Collision", PanelMode::Lines, 12);
                {
                    ImGui::TextUnformatted("Collider");
                    ImGui::SameLine();

                    const char* typeText = (data.type == ColliderType::AABB) ? "AABB" : (data.type == ColliderType::Sphere) ? "Sphere" : "OBB";
                    GuiUtility::Badge(typeText, ImVec4(0.20f, 0.55f, 1.00f, 1.0f));

                    ImGui::SameLine();
                    bool enabled = data.enabled;
                    if (ImGui::Checkbox("Enabled", &enabled))
                        SetEnabled(handle, enabled);

                    ImGui::SameLine();
                    ImGui::Text("Layer: 0x%08X", data.layerMask);
                }

                ImGui::Separator();

                // Change Types
                {
                    auto TypeButton = [&](const char* label, ColliderType targetType, const ImVec4& onColor, const ImVec4& offColor)
                        {
                            const bool isCurrent = (data.type == targetType);
                            ImGui::PushStyleColor(ImGuiCol_Button, isCurrent ? onColor : offColor);
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, isCurrent ? onColor :
                                ImVec4(offColor.x * 1.1f, offColor.y * 1.1f, offColor.z * 1.1f, offColor.w));
                            ImGui::PushStyleColor(ImGuiCol_ButtonActive, isCurrent ? onColor : offColor);
                            const bool clicked = ImGui::Button(label);
                            ImGui::PopStyleColor(3);
                            return clicked;
                        };

                    const ImVec4 onColor = ImVec4(0.24f, 0.72f, 0.38f, 1.0f);
                    const ImVec4 offColor = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);

                    ImGui::TextUnformatted("Type");
                    ImGui::SameLine();

                    // → AABB
                    if (TypeButton("AABB", ColliderType::AABB, onColor, offColor))
                    {
                        BoundingBox aabbLocal{};
                        if (data.type == ColliderType::Sphere)
                            aabbLocal = Utility::ToAABBFromSphere(data.sphere.centerLocal, data.sphere.radiusLocal);
                        else if (data.type == ColliderType::OBB)
                            aabbLocal = Utility::ToAABBFromOBB(data.obb.local);
                        else
                            aabbLocal = data.aabb.local;

                        SetAABB(handle, aabbLocal);
                    }

                    ImGui::SameLine();

                    // → Sphere
                    if (TypeButton("Sphere", ColliderType::Sphere, onColor, offColor))
                    {
                        _float3 centerLocal{};
                        float   radiusLocal = 0.5f;

                        if (data.type == ColliderType::AABB)
                        {
                            auto sphere = Utility::ToSphereFromAABB(data.aabb.local);
                            centerLocal = sphere.first;
                            radiusLocal = sphere.second;
                        }
                        else if (data.type == ColliderType::OBB)
                        {
                            BoundingBox aabbLocal = Utility::ToAABBFromOBB(data.obb.local);
                            auto sphere = Utility::ToSphereFromAABB(aabbLocal);
                            centerLocal = sphere.first;
                            radiusLocal = sphere.second;
                        }
                        else
                        {
                            centerLocal = data.sphere.centerLocal;
                            radiusLocal = data.sphere.radiusLocal;
                        }

                        SetSphere(handle, centerLocal, max(radiusLocal, 1e-6f));
                    }

                    ImGui::SameLine();

                    // → OBB
                    if (TypeButton("OBB", ColliderType::OBB, onColor, offColor))
                    {
                        BoundingOrientedBox obbLocal{};
                        if (data.type == ColliderType::AABB)
                            obbLocal = Utility::ToOBBFromAABB(data.aabb.local);

                        else if (data.type == ColliderType::Sphere)
                        {
                            BoundingBox aabbLocal = Utility::ToAABBFromSphere(data.sphere.centerLocal, data.sphere.radiusLocal);
                            obbLocal = Utility::ToOBBFromAABB(aabbLocal);
                        }
                        else
                            obbLocal = data.obb.local;

                        SetOBB(handle, obbLocal);
                    }
                }

                ImGui::Separator();

                if (data.type == ColliderType::AABB)
                {
                    _float3 centerLocal = data.aabb.local.Center;
                    _float3 extentsLocal = data.aabb.local.Extents;

                    ImGui::TextDisabled("AABB (Local)");
                    ImGui::DragFloat3("Center", &centerLocal.x, 0.01f);
                    if (ImGui::DragFloat3("Extents", &extentsLocal.x, 0.01f, 1e-6f, 1e6f))
                    {
                        extentsLocal.x = max(extentsLocal.x, 1e-6f);
                        extentsLocal.y = max(extentsLocal.y, 1e-6f);
                        extentsLocal.z = max(extentsLocal.z, 1e-6f);
                    }

                    if (ImGui::Button("Apply##AABB"))
                    {
                        BoundingBox aabbLocal(centerLocal, extentsLocal);
                        SetAABB(handle, aabbLocal);
                    }
                }
                else if (data.type == ColliderType::Sphere)
                {
                    _float3 centerLocal = data.sphere.centerLocal;
                    float   radiusLocal = data.sphere.radiusLocal;

                    ImGui::TextDisabled("Sphere (Local)");
                    ImGui::DragFloat3("Center", &centerLocal.x, 0.01f);
                    ImGui::DragFloat("Radius", &radiusLocal, 0.01f, 1e-6f, 1e6f);

                    if (ImGui::Button("Apply##Sphere"))
                        SetSphere(handle, centerLocal, max(radiusLocal, 1e-6f));

                }
                else
                {
                    _float3 centerLocal     = data.obb.local.Center;
                    _float3 extentsLocal    = data.obb.local.Extents;
                    _float4 orientationQuat = data.obb.local.Orientation; 

                    ImGui::TextDisabled("OBB (Local)");
                    ImGui::DragFloat3("Center", &centerLocal.x, 0.01f);
                    if (ImGui::DragFloat3("Extents", &extentsLocal.x, 0.01f, 1e-6f, 1e6f))
                    {
                        extentsLocal.x = max(extentsLocal.x, 1e-6f);
                        extentsLocal.y = max(extentsLocal.y, 1e-6f);
                        extentsLocal.z = max(extentsLocal.z, 1e-6f);
                    }

                    if (ImGui::DragFloat4("Orientation (x,y,z,w)", &orientationQuat.x, 0.01f, -1.f, 1.f))
                    {
                        XMVECTOR quat = XMVector4Normalize(XMLoadFloat4(&orientationQuat));
                        XMStoreFloat4(&orientationQuat, quat);
                    }

                    if (ImGui::Button("Apply##OBB"))
                    {
                        BoundingOrientedBox obbLocal{};
                        obbLocal.Center = centerLocal;
                        obbLocal.Extents = extentsLocal;
                        obbLocal.Orientation = orientationQuat;
                        SetOBB(handle, obbLocal);
                    }
                }
                GuiUtility::EndPanel();
            }
            ImGui::PopID();
        });
#endif
}