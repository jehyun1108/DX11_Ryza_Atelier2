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

	float rotDeg  = 0.f;
	float pivotNX = 0.5f;
	float pivotNY = 0.5f;
	float alpha   = 0.f;

	float fillRatioX = 1.f;
	float fillRatioY = 1.f;

	UIFillMode fillMode = UIFillMode::Rect;
	UIFlipMode flipMode = UIFlipMode::None;
	UIMaskType maskType = UIMaskType::None;

	float srcU0 = 0.f;
	float srcV0 = 0.f;
	float srcU1 = 1.f;
	float srcV1 = 1.f;

	_float4 color;
};
struct UIVertex
{
	float posX, posY;
	float uvX, uvY;
	float fillX;
	float fillY;
	float mode;
	float alpha;
	float maskType;

	_float4 color;
};
struct MinimapScreenRect
{
	float centerX;
	float centerY;
	float radiusPx;
	float anchorX;
	float anchorY;
};
using ResolveTexture = function<ID3D11ShaderResourceView* (const wstring&)>;

NS_END