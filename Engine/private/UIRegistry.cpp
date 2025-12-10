#include "Enginepch.h"

#include "RenderTargetMinimap.h"

void UIRegistry::OnBoot()
{
	assets = &registry.Get<AssetSystem>();
}

UIInstance& UIRegistry::Ensure(const wstring& key, EntityID owner)
{
	if (auto it = instances.find(key); it != instances.end())
		return it->second;

	auto itSpec = archetypes.find(key);
	assert(itSpec != archetypes.end() && "UIRegistry::Ensure - archetype not registered");

	const UIArchetypeSpec* spec = &itSpec->second;

	UIInstance inst{};
	inst.archetypeKey = key;
	inst.spec         = spec;

	if (inst.spec->initPosX) inst.localX = *spec->initPosX;
	if (inst.spec->initPosY) inst.localY = *spec->initPosY;
	inst.parentEntity = owner;

	inst.useScissor  = spec->useScissor;
	inst.selfEnabled = spec->startEnabled;
	inst.flipMode    = spec->flipMode;

	auto [iter, ok] = instances.emplace(key, inst);
	assert(ok);
	return iter->second;
}

void UIRegistry::SetParent(const wstring& key, EntityID parent)
{
	auto it = instances.find(key);
	assert(it != instances.end());
	it->second.parentEntity = parent;
}

void UIRegistry::SetLocalPos(const wstring& key, float x, float y)
{
	auto it = instances.find(key);
	assert(it != instances.end());
	it->second.localX = x;
	it->second.localY = y;
}

void UIRegistry::GetLocalPos(const wstring& key, float& outX, float& outY) const
{
	auto it = instances.find(key);
	assert(it != instances.end());
	const UIInstance& inst = it->second;
	outX = inst.localX;
	outY = inst.localY;
}

void UIRegistry::SetEnabled(const wstring& key, bool enabled)
{
	auto it = instances.find(key);
	assert(it != instances.end());
	it->second.selfEnabled = enabled;
}

void UIRegistry::SetScissor(const wstring& key, bool use, const UIRect& rect)
{
	auto it = instances.find(key);
	assert(it != instances.end());
	it->second.useScissor = use;
	it->second.scissorRect = rect;
}

void UIRegistry::SetWidgetTexture(const wstring& key, const wstring& texKey)
{
	auto it = instances.find(key);
	assert(it != instances.end());
	it->second.overrideKey = texKey;
}

void UIRegistry::SetFillRatioX(const wstring& key, float ratio)
{
	UIInstance& inst = Ensure(key);
	inst.fillRatioX = Utility::Saturate(ratio);
}

void UIRegistry::SetFillRatioY(const wstring& key, float ratio)
{
	UIInstance& inst = Ensure(key);
	inst.fillRatioY = Utility::Saturate(ratio);
}

void UIRegistry::SetFlipMode(const wstring& key, UIFlipMode mode)
{
	auto it = instances.find(key);
	assert(it != instances.end());
	it->second.flipMode = mode;
}

void UIRegistry::SetZOrder(const wstring& key, int zOrder)
{
	auto it = instances.find(key);
	assert(it != instances.end());
	it->second.zOrder = zOrder;
}

void UIRegistry::SetText(const wstring& key, const wstring& text)
{
	auto it = instances.find(key);
	assert(it != instances.end());
	it->second.text = text;
}

void UIRegistry::SetRotation(const wstring& key, float rad)
{
	auto it = instances.find(key);
	assert(it != instances.end());
	it->second.animRotDeg = rad;
}

int UIRegistry::GetZOrder(const wstring& key)
{
	auto it = instances.find(key);
	assert(it != instances.end());
	const UIInstance& inst = it->second;
	assert(inst.spec);
	const int base = inst.spec->zOrder;
	return base + inst.zOrder;
}

void UIRegistry::CollectForContext(UIContext context, vector<const UIInstance*>& out)
{
	for (auto& kv : instances)
	{
		const UIInstance& inst = kv.second;
		assert(inst.spec);
		if (inst.spec->context != context) continue;
		if (inst.selfEnabled)
			out.push_back(&inst);  
	}
}

pair<float, float> UIRegistry::GetOrCacheTexSize(const wstring& texKey)
{
	auto it = sizeCache.find(texKey);
	if (it != sizeCache.end()) return it->second;

	if (texKey == L"field_minimap_in")
	{
		auto& rtMini = registry.Get<RenderTargetMinimap>();
		const auto& vp = rtMini.GetViewport();
		pair<float, float> wh{ vp.Width, vp.Height };
		sizeCache.emplace(texKey, wh);
		return wh;
	}

	shared_ptr<Texture> tex = assets->GetTexture(texKey);
	assert(tex && "UIRegistry::GetOrCacheTexSize - texture not found");

	pair<float, float> widthHeight = 
	{
		static_cast<float>(tex->GetWidth()),
		static_cast<float>(tex->GetHeight())
	};
	sizeCache.emplace(texKey, widthHeight);
	return widthHeight;
}

UIInstance& UIRegistry::EnsureClone(const wstring& key, const wstring& instanceKey, EntityID owner)
{
	auto itSpec = archetypes.find(key);
	assert(itSpec != archetypes.end() && "EnsureClone - archetype not registered");
	const UIArchetypeSpec* spec = &itSpec->second;

	if (auto it = instances.find(instanceKey); it != instances.end())
		return it->second;

	UIInstance inst{};
	inst.archetypeKey = key;
	inst.spec = spec;
	if (spec->initPosX) inst.localX = *spec->initPosX;
	if (spec->initPosY) inst.localY = *spec->initPosY;
	inst.parentEntity = owner;
	inst.useScissor = spec->useScissor;
	inst.selfEnabled = spec->startEnabled;
	inst.flipMode = spec->flipMode;

	auto [iter, ok] = instances.emplace(instanceKey, inst);
	assert(ok);
	return iter->second;
}

void UIRegistry::SetScale(const wstring& key, float sx, float sy)
{
	auto it = instances.find(key);
	assert(it != instances.end());

	UIInstance& inst = it->second;
	inst.animScaleX = sx;
	inst.animScaleY = sy;
}