#pragma once

#include "UIRegistryData.h"

NS_BEGIN(Engine)

class ENGINE_DLL UIRegistry : public ISystem
{
public:
	explicit UIRegistry(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void        RegisterArchetype(const wstring& key, const UIArchetypeSpec& spec) { archetypes[key] = spec; }
	UIInstance& Ensure(const wstring& key, EntityID owner = invalidEntity);

	void SetParent(const wstring&   key, EntityID parent);
	void SetLocalPos(const wstring& key, float x, float y);
	void GetLocalPos(const wstring& key, float& outX, float& outY) const;

	void SetEnabled(const wstring& key, bool enabled);
	void SetScissor(const wstring& key, bool use, const UIRect& rect);
	void SetWidgetTexture(const wstring& key, const wstring& texKey);
	void SetFillRatioX(const wstring& key, float ratio);
	void SetFillRatioY(const wstring& key, float ratio);
	void SetFlipMode(const wstring& key, UIFlipMode mode);
	void SetZOrder(const wstring& key, int zOrder);
	void SetText(const wstring& key, const wstring& text);
	void SetRotation(const wstring& key, float rad);
	void SetScale(const wstring& key, float scaleX, float scaleY);

	int  GetZOrder(const wstring& key);
	void CollectForContext(UIContext context, vector<const UIInstance*>& out);

	pair<float, float> GetOrCacheTexSize(const wstring& texKey);
	UIInstance& EnsureClone(const wstring& key, const wstring& instanceKey, EntityID owner = invalidEntity);

	const unordered_map<wstring, UIArchetypeSpec>& GetArchetypes() const { return archetypes; }
	const unordered_map<wstring, UIInstance>&      GetInstances()  const { return instances; }

private:
	unordered_map<wstring, UIArchetypeSpec>    archetypes;
	unordered_map<wstring, UIInstance>         instances;
	unordered_map<wstring, pair<float, float>> sizeCache;

private:
	SystemRegistry& registry;
	AssetSystem*    assets{};
};

NS_END