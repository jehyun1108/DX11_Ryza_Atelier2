#include "Enginepch.h"
#include "FontSystem.h"

void FontSystem::RegisterFont(const wstring& fontKey, const FontDesc& desc)
{
	auto result = fonts.emplace(fontKey, desc);
	assert(result.second && "FontSystem::RegisterFont - fontKey already registered");
}

const FontDesc& FontSystem::GetFont(const wstring& fontKey) const
{
	auto it = fonts.find(fontKey);
	assert(it != fonts.end() && "FontSystem::GetFont - fontKey not registered");
	return it->second;
}

const Glyph& FontSystem::GetGlyph(const wstring& fontKey, char32_t codePoint) const
{
	const FontDesc& font = GetFont(fontKey);
	auto it = font.glyphs.find(codePoint);
	assert(it != font.glyphs.end() && "FontSystem::GetGlyph - glyph not found in font");
	return it->second;
}
