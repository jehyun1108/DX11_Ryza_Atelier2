#include "Enginepch.h"
#include "BattleTargetHUDPresenter.h"
#include "BattleAttributeSystem.h"
#include "BattleBoardPresenter.h"

void BattleTargetHUDPresenter::OnBoot()
{
	uiReg       = &registry.Get<UIRegistry>();
	uiAnimSys   = &registry.Get<UIAnimSystem>();
	targetSys   = &registry.Get<BattleTargetSystem>();
	timelineSys = &registry.Get<BattleTimelineSystem>();
	dataSys     = &registry.Get<CharacterDataSystem>();
	attrSys     = &registry.Get<BattleAttributeSystem>();
	sessionSys  = &registry.Get<BattleSessionSystem>();

	cfg.layout.barPos[0] = _float2{ 650.f, -500.f };
	cfg.layout.barPos[1] = _float2{ 750.f, -500.f };
	cfg.layout.barPos[2] = _float2{ 850.f, -500.f };

	cfg.layout.iconOffset   = _float2{ 0.f, -5.f };
	cfg.layout.hpBackOffset = _float2{ 0.f, 60.f };
	cfg.layout.hpFrontOffset = _float2{ -2.f, 55.f };
	cfg.layout.cursorOffset = _float2{ 0.f, -90.f };
	cfg.layout.labelOffset  = _float2{ 0.f, -50.f };

	cfg.globalKeys.cursor = L"target_cursor_global";
	cfg.globalKeys.label  = L"target_label_global";
	for (int i = 0; i < TargetHUDLayout::MaxSlots; ++i)
	{
		auto& keys = cfg.slotKeys[(size_t)i];

		keys.barBack = L"target_barback_"   + to_wstring(i);
		keys.hpBack  = L"target_hp_back_"   + to_wstring(i);
		keys.hpFront = L"target_hp_front_"  + to_wstring(i);
		keys.icon    = L"target_icon_slot_" + to_wstring(i);
		keys.ring    = L"target_ring_front_" + to_wstring(i);
	}
}

void BattleTargetHUDPresenter::Enter()
{
	EnsureWidgets();

	rt.leader = 0u;
	rt.curTarget = 0u;
	rt.focusedSlot = -1;
	rt.cursorBlinkTime = 0.f;

	ClearAllSlots();
	RefreshLeader();

	InitEnemySlots();
	RefreshCurTarget();
	UpdateSlotVisibility();
}

void BattleTargetHUDPresenter::Tick(float dt)
{
	RefreshLeader();
	RefreshCurTarget();

	UpdateTimelineRings(dt);
	UpdateHpBars(dt);
	UpdateCursorAndLabel(dt);
	UpdateSlotVisibility();
}

void BattleTargetHUDPresenter::Exit()
{
	ClearAllSlots();

	if (!cfg.globalKeys.cursor.empty())
		uiReg->SetEnabled(cfg.globalKeys.cursor, false);
	if (!cfg.globalKeys.label.empty())
		uiReg->SetEnabled(cfg.globalKeys.label, false);
}

void BattleTargetHUDPresenter::EnsureWidgets()
{
	const wstring baseBarBack = L"target_barback";
	const wstring baseHpBack  = L"target_hp_barback";
	const wstring baseHpFront = L"target_hp_barfront";
	const wstring baseIcon    = L"timeline_npc";   
	const wstring baseRing    = L"target_timeline_ring_front";
	const wstring baseCursor  = L"target_cursor";
	const wstring baseLabel   = L"target_letter";

	for (int i = 0; i < TargetHUDLayout::MaxSlots; ++i)
	{
		auto& keys = cfg.slotKeys[(size_t)i];

		uiReg->EnsureClone(baseBarBack, keys.barBack);
		uiReg->EnsureClone(baseHpBack, keys.hpBack);
		uiReg->EnsureClone(baseHpFront, keys.hpFront);
		uiReg->EnsureClone(baseIcon, keys.icon);
		uiReg->EnsureClone(baseRing, keys.ring);

		EnableSlotWidgets(i, false);
		uiReg->SetFillRatioX(keys.ring, 0.f);
	}

	if (!cfg.globalKeys.cursor.empty())
	{
		uiReg->EnsureClone(baseCursor, cfg.globalKeys.cursor);
		uiReg->SetEnabled(cfg.globalKeys.cursor, false);
	}
	if (!cfg.globalKeys.label.empty())
	{
		uiReg->EnsureClone(baseLabel, cfg.globalKeys.label);
		uiReg->SetEnabled(cfg.globalKeys.label, false);
	}
}

void BattleTargetHUDPresenter::RefreshLeader()
{
	EntityID leader = timelineSys->GetLeader();
	if (leader == rt.leader)
		return;

	rt.leader = leader;
	rt.curTarget = 0u;
	rt.focusedSlot = -1;
}

void BattleTargetHUDPresenter::RefreshCurTarget()
{
	if (rt.leader == 0u)
	{
		rt.curTarget = 0u;
		rt.focusedSlot = -1;
		return;
	}

	EntityID target = targetSys->Get(rt.leader);
	if (target != 0u)
	{
		float hpRatio = attrSys->GetHpRatio01(target);
		if (hpRatio <= 0.f)
			target = 0u;
	}
	if (target == 0u)
	{
		rt.curTarget = 0u;
		rt.focusedSlot = -1;
		return;
	}

	rt.curTarget = target;

	int slotIdx = FindSlotIdx(target);
	if (slotIdx < 0)
	{
		slotIdx = FindEmptySlot();
		if (slotIdx >= 0)
			AssignSlot(slotIdx, target);
	}

	rt.focusedSlot = slotIdx;
}

void BattleTargetHUDPresenter::UpdateHpBars(float dt)
{
	constexpr float deadFadeDuration = 1.0f;

	for (int i = 0; i < TargetHUDLayout::MaxSlots; ++i)
	{
		auto& slot = rt.slots[(size_t)i];
		if (!slot.active)
			continue;

		const auto& keys = cfg.slotKeys[(size_t)i];

		if (slot.fadingOut)
		{
			slot.fadeTimer += dt;
			float t = clamp(slot.fadeTimer / deadFadeDuration, 0.f, 1.f);
			slot.fadeAlpha = 1.f - t;

			uiAnimSys->SetOpacity(keys.barBack, slot.fadeAlpha);
			uiAnimSys->SetOpacity(keys.hpBack, slot.fadeAlpha);
			uiAnimSys->SetOpacity(keys.hpFront, slot.fadeAlpha);
			uiAnimSys->SetOpacity(keys.icon, slot.fadeAlpha);
			uiAnimSys->SetOpacity(keys.ring, slot.fadeAlpha);

			if (t >= 1.f)
				ClearSlot(i);
			continue;
		}

		float ratio = attrSys->GetHpRatio01(slot.enemy);
		slot.hpRatio = ratio;

		float target  = slot.hpRatio;
		float current = slot.hpShown;
		float diff    = target - current;
		float step    = cfg.anim.hpFollowSpeed * dt;

		if (fabsf(diff) <= step)
			current = target;
		else
			current += (diff > 0.f ? step : -step);

		current = clamp(current, 0.f, 1.f);
		slot.hpShown = current;

		SetSlotHpFront(i, current);

		uiAnimSys->SetOpacity(keys.barBack, 1.f);
		uiAnimSys->SetOpacity(keys.hpBack,  1.f);
		uiAnimSys->SetOpacity(keys.hpFront, 1.f);
		uiAnimSys->SetOpacity(keys.icon,    1.f);
		uiAnimSys->SetOpacity(keys.ring,    1.f);

		if (ratio <= 0.f && current <= 0.f)
		{
			slot.fadingOut = true;
			slot.fadeTimer = 0.f;
			slot.fadeAlpha = 1.f;
		}
	}
}

void BattleTargetHUDPresenter::UpdateTimelineRings(float dt)
{
	for (int i = 0; i < TargetHUDLayout::MaxSlots; ++i)
	{
		auto& slot = rt.slots[(size_t)i];
		if (!slot.active) continue;

		BattleTeam team{};
		int        timelineSlot = 0;
		const auto& unit = timelineSys->GetUnitStateByEntity(slot.enemy, team, timelineSlot);

		float ratio = 0.f;
		if (unit.ATB.maxValue > 0.f)
			ratio = clamp(unit.ATB.curValue / unit.ATB.maxValue, 0.f, 1.f);

		const auto& keys = cfg.slotKeys[(size_t)i];
		uiReg->SetFillRatioX(keys.ring, ratio);  
	}
}

void BattleTargetHUDPresenter::UpdateCursorAndLabel(float dt)
{
	if (rt.focusedSlot < 0 || rt.curTarget == 0u)
	{
		if (!cfg.globalKeys.cursor.empty())
			uiReg->SetEnabled(cfg.globalKeys.cursor, false);
		if (!cfg.globalKeys.label.empty())
			uiReg->SetEnabled(cfg.globalKeys.label, false);
		return;
	}

	const auto& slot = rt.slots[(size_t)rt.focusedSlot];

	if (!slot.active || slot.fadingOut)
	{
		if (!cfg.globalKeys.cursor.empty())
			uiReg->SetEnabled(cfg.globalKeys.cursor, false);
		if (!cfg.globalKeys.label.empty())
			uiReg->SetEnabled(cfg.globalKeys.label, false);
		return;
	}

	rt.cursorBlinkTime += dt;

	float phase = rt.cursorBlinkTime * cfg.anim.cursorBlinkFreq * XM_2PI;
	float s = 0.5f * (sinf(phase) + 1.f);
	float alpha = cfg.anim.cursorBlinkMin + (cfg.anim.cursorBlinkMax - cfg.anim.cursorBlinkMin) * s;

	if (!cfg.globalKeys.cursor.empty())
	{
		_float2 pos = GetCursorPos(rt.focusedSlot);
		uiReg->SetLocalPos(cfg.globalKeys.cursor, pos.x, pos.y);
		uiReg->SetEnabled(cfg.globalKeys.cursor, true);
		uiAnimSys->SetOpacity(cfg.globalKeys.cursor, alpha);
	}
	if (!cfg.globalKeys.label.empty())
	{
		_float2 pos = GetLabelPos(rt.focusedSlot);
		uiReg->SetLocalPos(cfg.globalKeys.label, pos.x, pos.y);
		uiReg->SetEnabled(cfg.globalKeys.label, true);
	}
}

void BattleTargetHUDPresenter::UpdateSlotVisibility()
{
	for (int i = 0; i < TargetHUDLayout::MaxSlots; ++i)
		EnableSlotWidgets(i, rt.slots[(size_t)i].active);
}

int BattleTargetHUDPresenter::FindSlotIdx(EntityID enemy) const
{
	for (int i = 0; i < TargetHUDLayout::MaxSlots; ++i)
	{
		const auto& slot = rt.slots[(size_t)i];
		if (slot.active && slot.enemy == enemy)
			return i;
	}
	return -1;
}

int BattleTargetHUDPresenter::FindEmptySlot() const
{
	for (int i = 0; i < TargetHUDLayout::MaxSlots; ++i)
	{
		if (!rt.slots[(size_t)i].active)
			return i;
	}
	return -1;
}

void BattleTargetHUDPresenter::AssignSlot(int slotIdx, EntityID enemy)
{
	auto& slot = rt.slots[(size_t)slotIdx];
	slot.enemy = enemy;
	slot.active = true;

	float ratio = attrSys->GetHpRatio01(enemy);
	slot.hpRatio = ratio;
	slot.hpShown = ratio;

	slot.fadingOut = false;
	slot.fadeTimer = 0.f;
	slot.fadeAlpha = 1.f;

	const auto& keys = cfg.slotKeys[(size_t)slotIdx];

	_float2 barPos     = GetBarPos(slotIdx);
	_float2 iconPos    = GetIconPos(slotIdx);
	_float2 hpBackPos  = GetHpBackPos(slotIdx);
	_float2 hpFrontPos = GetHpFrontPos(slotIdx);
	_float2 ringPos    = GetRingPos(slotIdx);

	uiReg->SetLocalPos(keys.barBack, barPos.x,     barPos.y);
	uiReg->SetLocalPos(keys.hpBack,  hpBackPos.x,  hpBackPos.y);
	uiReg->SetLocalPos(keys.hpFront, hpFrontPos.x, hpFrontPos.y);
	uiReg->SetLocalPos(keys.icon,    iconPos.x,    iconPos.y);
	uiReg->SetLocalPos(keys.ring,    ringPos.x,    ringPos.y);

	uiReg->SetWidgetTexture(keys.icon, L"timeline_npc");

	EnableSlotWidgets(slotIdx, true);
	SetSlotHpFront(slotIdx, slot.hpShown);

	BattleTeam team{};
	int        timelineSlot = 0;
	const auto& unit = timelineSys->GetUnitStateByEntity(enemy, team, timelineSlot);

	float atbRatio = 0.f;
	if (unit.ATB.maxValue > 0.f)
		atbRatio = clamp(unit.ATB.curValue / unit.ATB.maxValue, 0.f, 1.f);
	uiReg->SetFillRatioX(keys.ring, atbRatio);

	uiAnimSys->SetOpacity(keys.barBack, 1.f);
	uiAnimSys->SetOpacity(keys.hpBack,  1.f);
	uiAnimSys->SetOpacity(keys.hpFront, 1.f);
	uiAnimSys->SetOpacity(keys.icon,    1.f);
	uiAnimSys->SetOpacity(keys.ring,    1.f);
}

void BattleTargetHUDPresenter::InitEnemySlots()
{
	ClearAllSlots();
	
	const auto& enemies = sessionSys->GetEnemies();

	int slot = 0;
	for (int i = 0; i < enemies.memberCount && slot < TargetHUDLayout::MaxSlots; ++i)
	{
		EntityID enemy = enemies.members[i];
		if (enemy == 0u) continue;

		AssignSlot(slot, enemy);
		++slot;
	}
}

void BattleTargetHUDPresenter::ClearSlot(int slotIdx)
{
	rt.slots[(size_t)slotIdx] = TargetHUDSlotRuntime{};
	EnableSlotWidgets(slotIdx, false);
}

void BattleTargetHUDPresenter::ClearAllSlots()
{
	for (int i = 0; i < TargetHUDLayout::MaxSlots; ++i)
		ClearSlot(i);
}

void BattleTargetHUDPresenter::EnableSlotWidgets(int slotIdx, bool on)
{
	const auto& keys = cfg.slotKeys[(size_t)slotIdx];

	uiReg->SetEnabled(keys.barBack, on);
	uiReg->SetEnabled(keys.hpBack, on);
	uiReg->SetEnabled(keys.hpFront, on);
	uiReg->SetEnabled(keys.icon, on);
	uiReg->SetEnabled(keys.ring, on);
}

void BattleTargetHUDPresenter::SetSlotHpFront(int slotIdx, float ratio)
{
	const auto& keys = cfg.slotKeys[(size_t)slotIdx];
	float r = clamp(ratio, 0.f, 1.f);
	uiReg->SetFillRatioX(keys.hpFront, r);
}

_float2 BattleTargetHUDPresenter::GetHpBackPos(int slotIdx) const
{
	_float2 base = GetBarPos(slotIdx);
	return _float2{
		base.x + cfg.layout.hpBackOffset.x,
		base.y + cfg.layout.hpBackOffset.y
	};
}

_float2 BattleTargetHUDPresenter::GetHpFrontPos(int slotIdx) const
{
	_float2 base = GetBarPos(slotIdx);
	return _float2{
		base.x + cfg.layout.hpFrontOffset.x,
		base.y + cfg.layout.hpFrontOffset.y
	};
}

_float2 BattleTargetHUDPresenter::GetBarPos(int slotIdx) const
{
	return cfg.layout.barPos[(size_t)slotIdx];
}

_float2 BattleTargetHUDPresenter::GetIconPos(int slotIdx) const
{
	_float2 base = GetBarPos(slotIdx);
	return _float2{ base.x + cfg.layout.iconOffset.x, base.y + cfg.layout.iconOffset.y };
}

_float2 BattleTargetHUDPresenter::GetCursorPos(int slotIdx) const
{
	_float2 base = GetBarPos(slotIdx);
	return _float2{ base.x + cfg.layout.cursorOffset.x, base.y + cfg.layout.cursorOffset.y };
}

_float2 BattleTargetHUDPresenter::GetLabelPos(int slotIdx) const
{
	_float2 base = GetBarPos(slotIdx);
	return _float2{ base.x + cfg.layout.labelOffset.x, base.y + cfg.layout.labelOffset.y };
}

_float2 BattleTargetHUDPresenter::GetRingPos(int slotIdx) const
{
	return GetIconPos(slotIdx);
}

void BattleTargetHUDPresenter::OnLeaderChanged(const BattleEvent& e)
{
	RefreshLeader();
	RefreshCurTarget();
}