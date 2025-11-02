#pragma once

NS_BEGIN(Engine)

enum class UIScalePolicy { None, FitWidth, FitHeight, Stretch, KeepPixel };
enum class UIBlendConvention { PM_ALPHA };

struct NineSlice 
{ 
	int left   = 0; 
	int right  = 0;
	int top    = 0;
	int bottom = 0;
};

struct NormalizedPivot
{
	float px = 0.5f;
	float py = 0.5f;
};

struct UITextureMeta
{
	wstring         texKey;
	NormalizedPivot pivot{ 0.5f, 0.5f };
	NineSlice       nineSlice{};
	UIScalePolicy   scalePolicy = UIScalePolicy::None;
	int pixelWidth  = 0;
	int pixelHeight = 0;
};

NS_END