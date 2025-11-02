#pragma once

NS_BEGIN(Engine)
class Texture;

struct UIRect
{
	float x      = 0.f;
	float y      = 0.f;
	float width  = 0.f;
	float height = 0.f;
};

struct UIColor
{
	float r = 1.f;
	float g = 1.f;
	float b = 1.f;
	float a = 1.f;
};

struct UIDrawImage
{
	Texture* texture{};
	UIRect   dstRectPixels{};
	UIRect   srcUv{ 0.f, 0.f, 1.f, 1.f };
	UIColor  modulate{};
	float    zOrder{};
	bool     enableScissor = false;
	UIRect   scissor{};
};

struct UIScaleInput
{
	UIRect parentRect{};
	UITextureMeta meta;
	NormalizedPivot anchor{ 0.f, 0.f };
	float desiredWidth = 0.f;
	float desiredHeight = 0.f;
};

struct NineSliceQuads
{
	array<UIDrawImage, 9> images{};
	int count = 0;
};

struct UIDrawKey
{
	Texture* texture{};
	float    zOrder = 0.f;
};

inline bool operator<(const UIDrawKey& a, const UIDrawKey& b)
{
	if (a.texture != b.texture) return a.texture < b.texture;
	return a.zOrder < b.zOrder;
}

struct UIDrawItem
{
	UIDrawKey  key{};
	UIDrawImage image{};
};

NS_END