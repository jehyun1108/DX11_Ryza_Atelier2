#include "Enginepch.h"

Handle LightSystem::Create(EntityID owner, Handle transform, const LightProxy& proxy)
{
	Handle handle   = CreateComp(owner);
	auto& light     = *Get(handle);
	light           = {};
	light.transform = transform;
	light.proxy     = proxy;
	light.enabled   = true;
	return handle;
}

void LightSystem::Update(float dt)
{
	auto& tfSys = registry.Get<TransformSystem>();

	ForEachAlive([&](_uint, LightData& light) 
		{
			if (!light.enabled) return;

			const auto* tf = tfSys.Get(light.transform);
			if (!tf) return;

			XMStoreFloat4(&light.proxy.lightPos, _vec{ tf->pos.x, tf->pos.y, tf->pos.z , 1.f});

			const _vec look = tfSys.GetLook(light.transform);
			XMStoreFloat4(&light.proxy.lightDir, XMVector3Normalize(look));
		});
}

void LightSystem::ExtractLightProxies(vector<LightProxy>& out) const
{
	auto& tfSys = registry.Get<TransformSystem>();
	out.clear();

	ForEachAliveEx([&](Handle handle, EntityID id, const LightData& data)
		{
			if (!data.enabled) return;
			
			LightProxy proxy = data.proxy;
			if (const TransformData* tf = tfSys.Get(data.transform))
			{
				XMStoreFloat4(&proxy.lightPos, XMVectorSet(tf->pos.x, tf->pos.y, tf->pos.z, 1.f));
				_vec look = tfSys.GetLook(data.transform);
				XMStoreFloat4(&proxy.lightDir, XMVector3Normalize(look));
			}
			out.emplace_back(proxy);
		});
}

void LightSystem::SetEnabled(Handle handle, bool on)
{
	if (auto light = Get(handle))
		light->enabled = on;
}

void LightSystem::SetLightType(Handle handle, LIGHT type)
{
	if (auto light = Get(handle))
		light->proxy.type = ENUM(type);
}

void LightSystem::SetAmbient(Handle handle, _fvec color)
{
	if (auto light = Get(handle))
		XMStoreFloat4(&light->proxy.ambient, color);
}

void LightSystem::SetDiffuse(Handle handle, _fvec color)
{
	if (auto light = Get(handle))
		XMStoreFloat4(&light->proxy.diffuse, color);
}

void LightSystem::SetSpecular(Handle handle, _fvec color)
{
	if (auto light = Get(handle))
		XMStoreFloat4(&light->proxy.specular, color);
}

void LightSystem::SetRange(Handle handle, float range)
{
	if (auto light = Get(handle)) 
		light->proxy.range = max(0.f, range);
}

void LightSystem::SetSpotAngle(Handle handle, float angle)
{
	if (auto light = Get(handle))
		light->proxy.spotAngle = clamp(angle, 1e-4f, XM_PI - 1e-4f);
}

void LightSystem::SetDir(Handle handle, _fvec dir)
{
	if (auto light = Get(handle))
		XMStoreFloat4(&light->proxy.lightDir, XMVector3Normalize(dir));
}

const LightProxy* LightSystem::GetProxy(Handle handle) const
{
	return pool.Validate(handle) ? &pool.Get(handle)->proxy : nullptr;
}

void LightSystem::RenderGui(EntityID id)
{
#ifdef USE_IMGUI
	auto& tfSys = registry.Get<TransformSystem>();
	ForEachOwned(id, [&](Handle handle, LightData& light)
		{
			ImGui::PushID((int)handle.idx);

			if (ImGui::CollapsingHeader("Light"))
			{
				// Enabled
				bool enabled = light.enabled;
				if (ImGui::Checkbox("Enabled", &enabled))
					SetEnabled(handle, enabled);

				// Type
				static const char* types[] = { "Directional", "Point", "Spot" };
				int typeIdx = 0;
				switch ((LIGHT)light.proxy.type)
				{
				case LIGHT::DIRECTIONAL: typeIdx = 0; break;
				case LIGHT::POINT:       typeIdx = 1; break;
				case LIGHT::SPOT:        typeIdx = 2; break;
				default:                 typeIdx = 0; break;
				}
				if (ImGui::Combo("Type", &typeIdx, types, IM_ARRAYSIZE(types)))
				{
					SetLightType(handle, (typeIdx == 0) ? LIGHT::DIRECTIONAL :
						(typeIdx == 1) ? LIGHT::POINT : LIGHT::SPOT);
				}

				// Color
				ImGui::SeparatorText("Color");
				{
					float ambient[4] = { light.proxy.ambient.x, light.proxy.ambient.y, light.proxy.ambient.z, light.proxy.ambient.w };
					if (ImGui::ColorEdit4("Ambient", ambient))
						SetAmbient(handle, XMVectorSet(ambient[0], ambient[1], ambient[2], ambient[3]));

					float diffuse[4] = { light.proxy.diffuse.x, light.proxy.diffuse.y, light.proxy.diffuse.z, light.proxy.diffuse.w };
					if (ImGui::ColorEdit4("Diffuse", diffuse))
						SetDiffuse(handle, XMVectorSet(diffuse[0], diffuse[1], diffuse[2], diffuse[3]));

					float specular[4] = { light.proxy.specular.x, light.proxy.specular.y, light.proxy.specular.z, light.proxy.specular.w };
					if (ImGui::ColorEdit4("Specular", specular))
						SetSpecular(handle, XMVectorSet(specular[0], specular[1], specular[2], specular[3]));
				}
				
				// Type-specific params
				ImGui::SeparatorText("Params");
				const LIGHT curType = (LIGHT)light.proxy.type;

				if (curType == LIGHT::POINT || curType == LIGHT::SPOT)
				{
					float range = light.proxy.range;
					if (ImGui::DragFloat("Range", &range, 0.1f, 0.0f, 1e6f, "%.3f"))
						SetRange(handle, max(0.0f, range));
				}
				
				if (curType == LIGHT::SPOT)
				{
					float spotDeg = XMConvertToDegrees(light.proxy.spotAngle);
					if (ImGui::DragFloat("Spot Angle", &spotDeg, 0.1f, 0.01f, 179.0f))
					{
						float rad = XMConvertToRadians(spotDeg);
						SetSpotAngle(handle, clamp(rad, 1e-4f, XM_PI - 1e-4f));
					}
				}

				if (curType == LIGHT::DIRECTIONAL || curType == LIGHT::SPOT)
				{
					ImGui::SeparatorText("Direction");
					ImGui::Text("Dir: (%.3f, %.3f, %.3f)", light.proxy.lightDir.x, light.proxy.lightDir.y, light.proxy.lightDir.z);

					if (ImGui::SmallButton("Use Transform Look"))
					{
						_vec look = tfSys.GetLook(light.transform);
						SetDir(handle, look);
					}
					ImGui::SameLine();
					if (ImGui::SmallButton("World Down"))
						SetDir(handle, XMVectorSet(0.f, -1.f, 0.f, 0.f));
				}
			}
			ImGui::PopID();
		});
#endif
}