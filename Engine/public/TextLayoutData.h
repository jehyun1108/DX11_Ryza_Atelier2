#pragma once

NS_BEGIN(Engine)

struct TextLayoutDesc
{
	wstring fontKey;
	wstring text;

	float originX = 0.f;
	float originY = 0.f;
	float scale   = 1.f;
	int   zOrder  = 0;
	float alpha   = 1.f;

	bool useOutline = false;
	float outlinePx = 2.f;
	_float4 textColor = { 1, 1, 1, 1 };
	_float4 outlineColor = { 0, 0, 0, 1 };
};

struct TextBounds
{
	float width;
	float height;
};

NS_END