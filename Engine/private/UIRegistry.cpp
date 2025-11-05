#include "Enginepch.h"

UIInstance& UIRegistry::Ensure(const wstring& archetypeKey, EntityID owner)
{
	if (auto it = instances.find(archetypeKey); it != instances.end()) 
		return it->second;

	auto itSpec = archetypes.find(archetypeKey);
	assert(itSpec != archetypes.end() && "UIRegistry::Ensure - archetype not registered");
	if (itSpec == archetypes.end())
	{
		static UIInstance dummy{};
		return dummy;
	}

	const UIArchetypeSpec* spec = &itSpec->second;

	UIInstance inst{};
	inst.archetypeKey = archetypeKey;
	inst.spec         = spec;

	if (inst.spec->initPosX) inst.localX = *spec->initPosX;
	if (inst.spec->initPosY) inst.localY = *spec->initPosY;
	inst.parentEntity = owner;

	inst.useScissor  = spec->useScissor;
	inst.selfEnabled = spec->startEnabled;

	auto [iter, ok] = instances.emplace(archetypeKey, inst);
	return iter->second;
}

void UIRegistry::SetParent(const wstring& archetypeKey, EntityID parent)
{
	instances.at(archetypeKey).parentEntity = parent;
}

void UIRegistry::SetLocalPos(const wstring& archetypeKey, float x, float y)
{
	auto& inst = instances.at(archetypeKey);
	inst.localX = x;
	inst.localY = y;
}

void UIRegistry::SetEnabled(const wstring& archetypeKey, bool enabled)
{
	instances.at(archetypeKey).selfEnabled = enabled;
}

void UIRegistry::SetScissor(const wstring& archetypeKey, bool use, const UIRect& rect)
{
	auto& inst       = instances.at(archetypeKey);
	inst.useScissor  = use;
	inst.scissorRect = rect;
}

void UIRegistry::SetWidgetTexture(const wstring& archetypeKey, const wstring& texKey)
{
	auto& inst = Ensure(archetypeKey);
	inst.overrideKey = texKey;
}

void UIRegistry::SetZOrder(const wstring& archetypeKey, int zOrder)
{
	auto& inst = instances.at(archetypeKey);
	inst.zOrder = zOrder;
}

int UIRegistry::GetZOrder(const wstring& archetypeKey)
{
	auto it = instances.find(archetypeKey);
	if (it == instances.end()) return 0;
	const UIInstance& inst = it->second;
	const int base = inst.spec ? inst.spec->zOrder : 0;
	return base + inst.zOrder;
}

void UIRegistry::CollectForContext(UIContext context, vector<const UIInstance*>& out)
{
	out.clear();
	out.reserve(instances.size());

	for (auto& [key, inst] : instances)
	{
		const UIArchetypeSpec* spec = inst.spec;
		if (!spec) continue;

		if (spec->context != context) continue;

		bool visible = inst.selfEnabled;
		if (visible)
			out.push_back(&inst);
	}
}

pair<float, float> UIRegistry::GetOrCacheTexSize(const wstring& texKey)
{
	auto it = sizeCache.find(texKey);
	if (it != sizeCache.end()) return it->second;

	shared_ptr<Texture> texture = assets.GetTexture(texKey);
	if (!texture) return { 1.f, 1.f };

	pair<float, float> widthHeight = {
		static_cast<float>(texture->GetWidth()),
		static_cast<float>(texture->GetHeight())
	};
	sizeCache.emplace(texKey, widthHeight);
	return widthHeight;
}