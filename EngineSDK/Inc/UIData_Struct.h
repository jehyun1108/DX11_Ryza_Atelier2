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



using ResolveTexture = function<const Texture* (const wstring&)>;

NS_END