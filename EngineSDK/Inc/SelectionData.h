#pragma once

NS_BEGIN(Engine)

// ------------- 선택 가능 여부 Layer ----------------------------
struct ENGINE_DLL SelectionData
{
	bool  selectable = true;
	_uint layerMask  = 0xFFFFFFFFu;
};

// ---------------- Hover 선택 상태 ------------------------------
struct ENGINE_DLL HoverState
{
	EntityID hovered = 0;
};

// ----------------- Drag / Snap --------------------------------
struct ENGINE_DLL DragPlane
{
	bool enabled = false;
	_float3 point{};
	_float3 normal = { 0, 1, 0 };
};

struct ENGINE_DLL SnapSpec
{
	bool     enabled = false;
	float    stepX   = 1.f;
	float    stepY   = 1.f;
	float    stepZ   = 1.f;
	_float3  origin{};
};

struct ENGINE_DLL SelectionContext
{
	// UI 차단
	bool wantCaptureMouse = false;

	// Ray
	bool rayValid   = false;
	bool fromScreen = true;

	// ScreenRay
	_float2        screenPos{};
	D3D11_VIEWPORT viewport{};
	Handle         cam{};

	// WorldRay (fromScreen = false 일때)
	_float3 worldRayOrigin{};
	_float3 worldRayDir{};

	// Filter
	_uint layerMask = 0xFFFFFFFFu;

	// Input
	bool mouseLeftDown = false;
	bool mouseLeftHeld = false;
	bool mouseLeftUp   = false;
	bool multiSelect   = false;

	// opts
	bool snapToGrid = false;

	// Drag / Snap
	DragPlane dragPlane{};
	SnapSpec  snap{};
};

struct ENGINE_DLL DragState
{
	bool    active = false;
	_float3 planeHitStart{}; // 드래그 시작 시 평면 교점
	unordered_map<EntityID, _float3> startPos;
};

NS_END