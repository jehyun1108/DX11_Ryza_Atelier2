#pragma once

#include "SelectionData.h"

NS_BEGIN(Engine)

class ENGINE_DLL SelectionSystem : public EntitySystem<SelectionData>, public IGuiRenderable
{
public:
	explicit SelectionSystem(SystemRegistry& registry) : EntitySystem(registry) {}
	
	Handle   Create(EntityID owner, bool selectable = true, _uint layerMask = 0xFFFFFFFFu);

	void  SetContext(const SelectionContext& context) { this->context = context; }
	const SelectionContext& GetContext() const { return context; }

	// Query
	HoverState                     GetHoverState()         const { return HoverState{ hovered }; }
	bool                           IsSelected(EntityID id) const { return selected.count(id) != 0; }
	const unordered_set<EntityID>& GetSelection()          const { return selected; }
	EntityID                       GetHovered()            const { return hovered; }

	// Control
	void ClearSelection();
	void SelectOnly(EntityID id);
	void ToggleSelect(EntityID id);

	// Tick
	void Update(float dt);
	void RenderGui(EntityID id) override;

private:
	void UpdateHover();
	void UpdateInput();
	void UpdateDrag(float dt);

	// Util
	static bool RayPlaneIntersect(const _float3& origin, const _float3& dir, const _float3& pos, const _float3& normal, _float3& outHit);
	static float Snap(float v, float step, float origin = 0.f);


private:
	SelectionContext        context{};
	DragState               drag;
	EntityID                hovered = 0;
	unordered_set<EntityID> selected;
};

NS_END