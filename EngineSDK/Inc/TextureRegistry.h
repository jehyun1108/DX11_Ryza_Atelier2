#pragma once

#include "TextureUtil.h"

NS_BEGIN(Engine)

class ENGINE_DLL TextureRegistry
{
public:
	void RegisterTexture(const wstring& logicalKey, const TextureMeta& meta);
	bool Exists(const wstring& logicalKey) const;

	const TextureMeta* GetTextureMeta(const wstring& logicalKey) const;
	// Tool -> ¿ªÀüÆÄ
	const TextureMeta* GetByNormalizedKey(const wstring& normalizedKey) const;

	void Remove(const wstring& logicalKey);

	void Clear() { table.clear(); }

private:
	unordered_map<wstring, TextureMeta> table;
};

NS_END
