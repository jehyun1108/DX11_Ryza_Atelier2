#pragma once

// 폰트 한 글자(글리프)에 대한 메트릭 정보
struct GlyphMetrics
{
	float offsetX = 0.f;
	float offsetY = 0.f;
	float advance = 0.f;
	float width   = 0.f;
	float height  = 0.f;
};
// 폰트 아틀라스에서의 UV영역
struct GlyphUV
{
	float u0 = 0.f;
	float v0 = 0.f;
	float u1 = 0.f;
	float v1 = 0.f;
};
// 실제 하나의 글리프 데이터: CodePoint + Metric + UV
struct Glyph
{
	char32_t     codePoint = 0;
	GlyphMetrics metrics{};
	GlyphUV      uv{};
};
// 폰트 하나에 대한 전체 정보
struct FontDesc
{
	wstring name;
	wstring atlasNameKey;

	float   lineHeight = 0.f;
	float   ascent     = 0.f;
	float   descent    = 0.f;

	unordered_map<char32_t, Glyph> glyphs;
};