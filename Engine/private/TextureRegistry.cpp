#include "Enginepch.h"

void TextureRegistry::RegisterTexture(const wstring& logicalKey, const TextureMeta& meta)
{
	auto key = Utility::Normalize(logicalKey);
	table[key] = meta;
}

bool TextureRegistry::Exists(const wstring& logicalKey) const
{
	auto key = Utility::Normalize(logicalKey);
	return table.find(key) != table.end();
}

const TextureMeta* TextureRegistry::GetTextureMeta(const wstring& logicalKey) const
{
	auto key = Utility::Normalize(logicalKey);
	auto it = table.find(key);
	return (it == table.end()) ? nullptr : &it->second;
}

const TextureMeta* TextureRegistry::GetByNormalizedKey(const wstring& normalizedKey) const
{
	auto it = table.find(normalizedKey);
	return (it == table.end()) ? nullptr : &it->second;
}

void TextureRegistry::Remove(const wstring& logicalKey)
{
	auto key = Utility::Normalize(logicalKey);
	table.erase(key);
}
