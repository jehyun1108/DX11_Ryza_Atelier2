#pragma once

NS_BEGIN(Engine)

enum class UISizeMode
{
	Original, Fixed, Ratio
};
enum class UIContext
{
	Field, Battle,
};
enum class UIAnchor
{
	TopLeft, TopCenter, TopRight, MidLeft, MidCenter, MidRight, BottomLeft, BottomCenter, BottomRight
};
enum class UIPivot
{
	TopLeft, TopCenter, TopRight, MidLeft, MidCenter, MidRight, BottomLeft, BottomCenter, BottomRight
};
enum class UILayer // 높을수록 먼저그림
{
	BackGround,
	BarBack,
	Widgets,
	FatalDrive,
	Overlay,
	Debug,
};
enum class UIWidgetState
{
	None, Hovered, Pressed, Disabled
};
enum class UIChannel
{
	Screen, Offscreen0, Offscreen1
};
enum class UITextAlignHorizontal
{
	Left, Center, Right
};
enum class UITextAlignVertical
{
	Top, Mid, Bottom
};
enum class UIDrawType
{
	Image, SolidRect, 
};
enum class UIEasing
{
	Linear, EaseOutCubic, EaseOutQuad
};
enum class UIVisibility
{
	Always, DuringCutScene, DuringSKill, DuringUltimate, Manual
};

NS_END