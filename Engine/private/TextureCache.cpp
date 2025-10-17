#include "Enginepch.h"

shared_ptr<Texture> TextureCache::Ensure(const wstring& logicalKey)
{
	const auto key = Utility::Normalize(logicalKey);
	if (auto it = cache.find(key); it != cache.end())
		return it->second;

	const TextureMeta* meta = registry.GetByNormalizedKey(key);
	if (!meta)
	{
		assert(false && "TextureCache::Ensure: key not found in registry");
		return nullptr;
	}

	auto texture = Texture::LoadFromFile(meta->fullPath, meta->colorSpace);
	if (!texture)
	{
		assert(false && "TextureCache::Ensure: Texture load failed");
		return nullptr;
	}
	cache.emplace(key, texture);
	return texture;
}

shared_ptr<Texture> TextureCache::Get(const wstring& logicalKey) const
{
	const auto key = Utility::Normalize(logicalKey);
	auto it = cache.find(key);
	return (it == cache.end()) ? nullptr : it->second;
}

void TextureCache::Erase(const wstring& logicalKey)
{
	const auto key = Utility::Normalize(logicalKey);
	cache.erase(key);
}
