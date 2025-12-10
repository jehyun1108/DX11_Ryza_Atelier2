#pragma once

NS_BEGIN(Engine)

enum class UISizeMode
{
	Original, Fixed, Ratio
};
enum class UIContext
{
	Always, Field, Battle, Loading, Logo, None
};
enum class UIAnchor
{
	TopLeft, TopCenter, TopRight, MidLeft, MidCenter, MidRight, BottomLeft, BottomCenter, BottomRight
};
enum class UIPivot
{
	TopLeft, TopCenter, TopRight, MidLeft, MidCenter, MidRight, BottomLeft, BottomCenter, BottomRight
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
enum class UIEasing
{
	Linear, EaseOutCubic, EaseOutQuad, EaseWave, EaseInOut
};
enum class UIVisibility
{
	Always, DuringCutScene, DuringSKill, DuringUltimate, Manual
};
enum class UIFillMode
{
	Rect, RingCW 
};
enum class UIFlipMode
{
	None, FlipX, FlipY, FlipXY
};
enum class UIMaskType
{
	None, Circle
};
enum class UIWidgetType
{
	Image, Text
};

NS_END