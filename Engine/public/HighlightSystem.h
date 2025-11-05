#pragma once

#include "HighlightData.h"

NS_BEGIN(Engine)

class ENGINE_DLL HighlightSystem : public ISystem
{
public:
	explicit HighlightSystem(SystemRegistry& registry) : registry(registry) {}

	void ClearFrame();
	void AddType(EntityID entityId, HighlightType type);
	void ResolvePolicy(const HighlightTable& table);
	bool ShouldOutline(EntityID entityId) const;
	const HighlightStyle* GetResolvedStyle(EntityID entityId) const;

private:
	HighlightStyle PickHigher(const HighlightStyle& a, const HighlightStyle& b);

private:
	SystemRegistry& registry;
	unordered_map<EntityID, _uint>          typeMap;
	unordered_map<EntityID, HighlightStyle> styleMap;
};

NS_END