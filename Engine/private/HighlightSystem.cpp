#include "Enginepch.h"

void HighlightSystem::ClearFrame()
{
	typeMap.clear();
	styleMap.clear();
}

void HighlightSystem::AddType(EntityID entityId, HighlightType type)
{
	if (entityId == 0) return;
	typeMap[entityId] |= ToBits(type);
}

void HighlightSystem::ResolvePolicy(const HighlightTable& table)
{
	styleMap.reserve(typeMap.size());

	for (const auto& pair : typeMap)
	{
		const EntityID entityId = pair.first;
		const _uint typeBits = pair.second;

		HighlightStyle best = {}; // Priority 0 = 표시x 기본값

		if (HasType(typeBits, HighlightType::Hover))
			best = PickHigher(best, table.hoverStyle);

		if (HasType(typeBits, HighlightType::Selected))
			best = PickHigher(best, table.selectedStyle);

		if (HasType(typeBits, HighlightType::Collision))
			best = PickHigher(best, table.collisionStyle);

		if (best.priority > 0)
			styleMap.emplace(entityId, best);
	}
}

bool HighlightSystem::ShouldOutline(EntityID entityId) const
{
	return styleMap.find(entityId) != styleMap.end();
}

const HighlightStyle* HighlightSystem::GetResolvedStyle(EntityID entityId) const
{
	auto it = styleMap.find(entityId);
	return (it != styleMap.end()) ? &it->second : nullptr;
}

HighlightStyle HighlightSystem::PickHigher(const HighlightStyle& a, const HighlightStyle& b)
{
	return (b.priority > a.priority) ? b : a;
}
