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

struct UIDrawItem
{
	int     zOrder = 0;
	UIRect  dstRect{};
	wstring texKey;

	bool   useScissor = false;
	UIRect scissorRect{};
};

struct UIVertex
{
	float posX, posY;
	float uvX, uvY;
};

struct UITween1D
{
	bool  playing  = false;
	float elapsed  = 0.f;
	float dur      = 0.25f;
	float start    = 0.f;
	float end      = 0.f;
	UIEasing easing = UIEasing::EaseOutCubic;
};

struct UIAnimChannels
{
	UITween1D offsetX, offsetY;

	UITween1D scaleX, scaleY;
	UITween1D opacity;
};

using ResolveTexture = function<const Texture* (const wstring&)>;

NS_END