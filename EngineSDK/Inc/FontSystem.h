#pragma once

#include "FontData.h"

NS_BEGIN(Engine)

class ENGINE_DLL FontSystem : public ISystem
{
public:
	explicit FontSystem(SystemRegistry& registry) : registry(registry) {}

	void RegisterFont(const wstring& fontKey, const FontDesc& desc);

	const FontDesc& GetFont(const wstring& fontKey) const;
	const Glyph&    GetGlyph(const wstring& fontKey, char32_t codePoint) const;

private:
	SystemRegistry& registry;
	unordered_map<wstring, FontDesc> fonts;
};

NS_END