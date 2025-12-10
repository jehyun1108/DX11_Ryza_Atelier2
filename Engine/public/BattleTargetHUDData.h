#pragma once

NS_BEGIN(Engine)

struct TargetHUDSlotKeys
{
	wstring barBack;
	wstring hpBack;
	wstring hpFront;
	wstring icon;
	wstring ring;
};
struct TargetHUDGlobalKeys
{
	wstring cursor;
	wstring label;
};
struct TargetHUDLayout
{
	static constexpr int MaxSlots = 3;
	
	array<_float2, MaxSlots> barPos{};
	
	_float2 iconOffset{ 0.f, 0.f };
	_float2 hpBackOffset{ 0.f, 40.f };
	_float2 hpFrontOffset{ 0.f, 40.f };
	_float2 cursorOffset{ 0.f, -120.f };
	_float2 labelOffset{ 0.f, 60.f };
};
struct TargetHUDAnimConfig
{
	float hpFollowSpeed   = 10.f;   
	float cursorBlinkFreq = 2.0f;  
	float cursorBlinkMin  = 0.3f;
	float cursorBlinkMax  = 1.0f;
};
struct TargetHUDConfig
{
	array<TargetHUDSlotKeys, TargetHUDLayout::MaxSlots> slotKeys{};
	TargetHUDGlobalKeys   globalKeys{};
	TargetHUDLayout       layout{};
	TargetHUDAnimConfig   anim{};
};
struct TargetHUDSlotRuntime
{
	EntityID enemy  = 0u;
	bool     active = false;

	float hpRatio = 1.f;
	float hpShown = 1.f;

	bool  fadingOut = false;
	float fadeTimer = 0.f;
	float fadeAlpha = 1.f;
};
struct TargetHUDRuntime
{
	EntityID leader    = 0u;
	EntityID curTarget = 0u;

	array<TargetHUDSlotRuntime, TargetHUDLayout::MaxSlots> slots{};

	int   focusedSlot     = -1;
	float cursorBlinkTime = 0.f;
};

NS_END