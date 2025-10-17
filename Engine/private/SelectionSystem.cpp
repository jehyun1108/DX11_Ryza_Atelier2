#include "Enginepch.h"

Handle SelectionSystem::Create(EntityID owner, bool selectable, _uint layerMask)
{
	Handle handle = CreateComp(owner);
	if (auto comp = Get(handle))
	{
		comp->selectable = selectable;
		comp->layerMask = layerMask;
	}
	return handle;
}

void SelectionSystem::ClearSelection()
{
	selected.clear();
}

void SelectionSystem::SelectOnly(EntityID id)
{
	selected.clear();
	if (id)
		selected.insert(id);
}

void SelectionSystem::ToggleSelect(EntityID id)
{
	if (!id) return;
	if (selected.count(id))
		selected.erase(id);
	else
		selected.insert(id);
}

void SelectionSystem::Update(float dt)
{
	GameInstance& game   = GameInstance::GetInstance();
	const _float2& mouse = game.GetMousePos();

	context.wantCaptureMouse = ImGui::GetIO().WantCaptureMouse;
	context.fromScreen       = true;
	context.screenPos        = { mouse.x, mouse.y };
	context.viewport         = game.GetViewport();
	context.cam              = registry.Get<CameraSystem>().GetMainCamHandle();
	context.rayValid         = true;

	// Buttons
	context.mouseLeftDown = game.KeyDown(KEY::LBUTTON);
	context.mouseLeftHeld = game.KeyPressing(KEY::LBUTTON);
	context.mouseLeftUp   = game.KeyRelease(KEY::LBUTTON);
	context.multiSelect   = game.KeyPressing(KEY::LCTRL);

// -------------------------------------------------------------
	UpdateHover();
	UpdateInput();
	UpdateDrag(dt);
}

void SelectionSystem::UpdateHover()
{
	hovered = 0;

	if (context.wantCaptureMouse) return;
	if (!context.rayValid)        return;

	_float3 origin{}, dir{};
	if (context.fromScreen)
	{
		auto camSys = registry.TryGet<CameraSystem>();
		if (!camSys || !camSys->Validate(context.cam)) return;

		_vec rayOrigin{}, rayDir{};
		camSys->CreateRayFromScreen(context.cam, context.screenPos, context.viewport, rayOrigin, rayDir);
		XMStoreFloat3(&origin, rayOrigin);
		XMStoreFloat3(&dir, XMVector3Normalize(rayDir));
	}
	else
	{
		origin = context.worldRayOrigin;
		XMStoreFloat3(&dir, XMVector3Normalize(XMLoadFloat3(&context.worldRayDir)));
	}

	// Picking
	auto pickSys = registry.TryGet<PickingSystem>();
	if (!pickSys) return;

	PickingHit hit{};
	if (pickSys->RaycastAll(origin, dir, context.layerMask, hit))
		hovered = hit.entity;
}

void SelectionSystem::UpdateInput()
{
	if (context.wantCaptureMouse) return;

	if (context.mouseLeftDown)
	{
		if (hovered == 0)
		{
			selected.clear();
			return;
		}
		SelectOnly(hovered);
	}
}

void SelectionSystem::UpdateDrag(float dt)
{
	if (context.wantCaptureMouse || !context.rayValid)
	{
		if (drag.active)
		{
			drag = {};
			context.dragPlane.enabled = false;
		}
		return;
	}

	// Ray
	_float3 origin{}, dir{};
	if (context.fromScreen)
	{
		auto camSys = registry.TryGet<CameraSystem>();
		if (!camSys || !camSys->Validate(context.cam)) return;

		_vec rayOrigin{}, rayDir{};
		camSys->CreateRayFromScreen(context.cam, context.screenPos, context.viewport, rayOrigin, rayDir);
		XMStoreFloat3(&origin, rayOrigin);
		XMStoreFloat3(&dir, XMVector3Normalize(rayDir));
	}
	else
	{
		origin = context.worldRayOrigin;
		XMStoreFloat3(&dir, XMVector3Normalize(XMLoadFloat3(&context.worldRayDir)));
	}

	auto& tfSys = registry.Get<TransformSystem>();

	if (!drag.active && context.mouseLeftDown)
	{
		// 단일 타겟
		if (selected.empty())
		{
			if (hovered == 0) return;
			SelectOnly(hovered);
		}
		else
		{
			if (hovered != 0)
				SelectOnly(hovered);
			else if (selected.size() > 1)
			{
				EntityID first = *selected.begin();
				selected.clear();
				selected.insert(first);
			}
		}

		// 평면 고정: 외부에서 켜지지 않았다면 XZ 평면으로 고정
		if (!context.dragPlane.enabled)
		{
			context.dragPlane.enabled = true;
			context.dragPlane.normal = Utility::Up3();
			context.dragPlane.point = _float3(0, 0, 0);
		}
		
		// 시작 히트점 계산 고정된 평면으로
		_float3 hit{};
		if (!RayPlaneIntersect(origin, dir, context.dragPlane.point, context.dragPlane.normal, hit))
		{
			const _vec vOrigin = XMLoadFloat3(&origin);
			const _vec vNormal = XMVector3Normalize(XMLoadFloat3(&context.dragPlane.normal));
			const float t0 = XMVectorGetX(XMVector3Dot(vNormal, XMVectorNegate(vOrigin)));
			_float3 newPoint{};
			XMStoreFloat3(&newPoint, XMVectorMultiplyAdd(vNormal, XMVectorReplicate(t0), vOrigin));
			context.dragPlane.point = newPoint;

			if (!RayPlaneIntersect(origin, dir, context.dragPlane.point, context.dragPlane.normal, hit))
				return;
		}

		drag.active = true;
		drag.planeHitStart = hit;
		drag.startPos.clear();

		EntityID target = hovered ? hovered : (!selected.empty()) ? *selected.begin() : 0;
		if (target)
		{
			if (auto tf = tfSys.GetByOwner(target))
				drag.startPos[target] = tf->pos;
		}
		return;
	}

	// Drag 실행
	if (drag.active && context.mouseLeftHeld)
	{
		if (!context.dragPlane.enabled) return;

		_float3 cur{};
		if (!RayPlaneIntersect(origin, dir, context.dragPlane.point, context.dragPlane.normal, cur)) 
			return;

		_float3 delta = { cur.x - drag.planeHitStart.x, cur.y - drag.planeHitStart.y, cur.z - drag.planeHitStart.z };

		// Snap
		if (context.snap.enabled)
		{
			delta.x = Snap(delta.x, context.snap.stepX, context.snap.origin.x);
			delta.y = Snap(delta.y, context.snap.stepY, context.snap.origin.y);
			delta.z = Snap(delta.z, context.snap.stepZ, context.snap.origin.z);
		}

		// 단일 대상
		for (auto& pair : drag.startPos)
		{
			EntityID id = pair.first;
			const _float3& start = pair.second;

			if (auto* tf = tfSys.GetByOwner(id))
			{
				tf->pos.x = start.x + delta.x;
				tf->pos.y = start.y + delta.y;
				tf->pos.z = start.z + delta.z;
				tf->dirty = true;
			}
		}
		return;
	}

	// Drag End
	if (drag.active && context.mouseLeftUp)
	{
		drag = {};
		context.dragPlane.enabled = false;
		return;
	}
}

bool SelectionSystem::RayPlaneIntersect(const _float3& origin, const _float3& dir, const _float3& pos, const _float3& normal, _float3& outHit)
{
	const _vec vOrigin = XMLoadFloat3(&origin);
	const _vec vDir    = XMVector3Normalize(XMLoadFloat3(&dir));
	const _vec vPos    = XMLoadFloat3(&pos);
	const _vec vNormal = XMVector3Normalize(XMLoadFloat3(&normal));

	const float dot = XMVectorGetX(XMVector3Dot(vNormal, vDir));
	if (fabsf(dot) < 1e-6f)
		return false; // 평행

	const float t = XMVectorGetX(XMVector3Dot(vNormal, vPos - vOrigin)) / dot;
	if (t < 0.f) return false;

	const _vec hit = XMVectorMultiplyAdd(vDir, XMVectorReplicate(t), vOrigin);
	XMStoreFloat3(&outHit, hit);
	return true;
}

float SelectionSystem::Snap(float v, float step, float origin)
{
	if (step <= 1e-6f) return v;
	const float rel = v - origin;
	const float q = roundf(rel / step);
	return origin + q * step;
}

void SelectionSystem::RenderGui(EntityID id) 
{
#ifdef USE_IMGUI
	if (ImGui::CollapsingHeader("Selection", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Hovered: %u", (_uint)hovered);
		ImGui::Text("Selected Count: %d", (int)selected.size());

		if (ImGui::TreeNode("Selected Entities"))
		{
			for (auto id : selected)
				ImGui::BulletText("%u", (_uint)id);
			ImGui::TreePop();
		}

		if (ImGui::Button("Clear Selection"))
			selected.clear();
	}

#endif
}