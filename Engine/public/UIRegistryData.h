#pragma once

// 등록 -> 인스턴스 -> 스냅샷
NS_BEGIN(Engine)

// 2. 등록용 정의 (ArchetypeSpec)
struct UIArchetypeSpec
{
	wstring texKey;

	int          zOrder     = 0;
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

	UIFillMode   fillMode   = UIFillMode::Rect;
	UIFlipMode   flipMode   = UIFlipMode::None;
	UIMaskType   maskType   = UIMaskType::None;
	UIWidgetType widgetType = UIWidgetType::Image;

	wstring               fontKey;
	wstring               defaultText;
	UITextAlignHorizontal alignH = UITextAlignHorizontal::Center;
	UITextAlignVertical   alignV = UITextAlignVertical::Mid;

	_float4 textColor    = _float4(1.f, 1.f, 1.f, 1.f);
	_float4 outlineColor = _float4(0.f, 0.f, 0.f, 1.f);
	bool    useOutline   = true;
	float   outlinePx    = 1.f;

	_float4 imageColor = _float4(1.f, 1.f, 1.f, 1.f);

	bool startEnabled = true;
};
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
	float    animScaleX  = 1.f;
	float    animScaleY  = 1.f;
	float    animAlpha   = 1.f;
	float    animRotDeg  = 0.f;

	float    fillRatioX = 1.f;
	float    fillRatioY = 1.f;

	int      zOrder = 0;
	UIFlipMode flipMode = UIFlipMode::None;

	wstring text;
	wstring fontKey;
};

NS_END