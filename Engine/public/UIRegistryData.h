#pragma once

// 등록 -> 인스턴스 -> 스냅샷
NS_BEGIN(Engine)

// 2. 등록용 정의 (ArchetypeSpec)
struct UIArchetypeSpec
{
	wstring texKey;

	UILayer      layer      = UILayer::Widgets;
	UIContext    context    = UIContext::Battle;
	UIVisibility visibility = UIVisibility::Always;

	bool        useScissor    = false;
	bool        isInteractive = false;

	UISizeMode sizeMode    = UISizeMode::Original;
	float      fixedWidth  = 0.f;
	float      fixedHeight = 0.f;
	float      ratioX      = 1.f;
	float      ratioY      = 1.f;

	UIPivot  pivot  = UIPivot::MidCenter;
	UIAnchor anchor = UIAnchor::MidCenter;

	optional<float> initPosX;
	optional<float> initPosY;

	bool startEnabled = true;
};

// Runtime Instance
struct UIInstance
{
	wstring archetypeKey;
	optional<wstring> overrideKey;
	const UIArchetypeSpec* spec = nullptr;

	EntityID parentEntity = invalidEntity;
	float    localX       = 0.f;
	float    localY       = 0.f;

	bool     selfEnabled     = true;
	bool     resolvedVisible = false;

	bool     useScissor  = false;
	UIRect   scissorRect = {};

	float    animOffsetX = 0.f;
	float    animOffsetY = 0.f;
};

NS_END