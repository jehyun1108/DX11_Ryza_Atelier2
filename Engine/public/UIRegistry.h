#pragma once

#include "UIRegistryData.h"

NS_BEGIN(Engine)

class ENGINE_DLL UIRegistry : public ISystem
{
public:
	explicit UIRegistry(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void        RegisterArchetype(const wstring& archetypeKey, const UIArchetypeSpec& spec) { archetypes[archetypeKey] = spec; }
	UIInstance& Ensure(const wstring& archetypeKey, EntityID owner = invalidEntity);

	void SetParent(const wstring&   archetypeKey, EntityID parent);
	void SetLocalPos(const wstring& archetypeKey, float x, float y);
	void SetEnabled(const wstring&  archetypeKey, bool enabled);
	void SetScissor(const wstring&  archetypeKey, bool use, const UIRect& rect);
	void SetWidgetTexture(const wstring& archetypeKey, const wstring& texKey);

	void SetZOrder(const wstring& archetypeKey, int zOrder);
	int  GetZOrder(const wstring& archetypeKey);

	// Field / Battle 기준으로 그릴 후보를 모아서 반환 -> visible 해석 (부모영향 + Context 허용) 여기서 해결
	void CollectForContext(UIContext context, vector<const UIInstance*>& out);

	pair<float, float> GetOrCacheTexSize(const wstring& texKey);

	const unordered_map<wstring, UIArchetypeSpec>& GetArchetypes() const { return archetypes; }
	const unordered_map<wstring, UIInstance>&      GetInstances()  const { return instances; }

private:
	SystemRegistry& registry;
	AssetSystem*    assets{};

	unordered_map<wstring, UIArchetypeSpec>    archetypes;
	unordered_map<wstring, UIInstance>         instances;
	unordered_map<wstring, pair<float, float>> sizeCache;
};

NS_END