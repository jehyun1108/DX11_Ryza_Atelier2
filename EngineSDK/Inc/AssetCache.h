#pragma once

NS_BEGIN(Engine)

template<typename TResourcePtr, typename TMeta, typename TRegistry>
class AssetCache
{
public:
	using LoaderFunc = function<TResourcePtr(const wstring& normalizedKey, const TMeta&)>;

	explicit AssetCache(TRegistry& registry, LoaderFunc loadFunc) : registry(registry), loadFunc(move(loadFunc)) {}

	void Prime(const wstring& logicalKey, TResourcePtr resource)
	{
		cache[Utility::Normalize(logicalKey)] = move(resource);
	}

	TResourcePtr Ensure(const wstring& logicalKey)
	{
		const wstring normalizedKey = Utility::Normalize(logicalKey);

		if (auto it = cache.find(normalizedKey); it != cache.end())
			return it->second;

		const TMeta* meta = registry.GetByNormalizedKey(normalizedKey);
		if (!meta)
		{
			meta = registry.Get(logicalKey);
			if (!meta) return nullptr;
		}

		TResourcePtr resource = loadFunc(normalizedKey, *meta);
		if (!resource) return nullptr;

		cache.emplace(normalizedKey, resource);
		return resource;
	}

	TResourcePtr Get(const wstring& logicalKey) const
	{
		const wstring normalizedKey = Utility::Normalize(logicalKey);
		auto it = cache.find(normalizedKey);
		return (it == cache.end()) ? nullptr : it->second;
	}

	void Erase(const wstring& logicalKey)
	{
		const wstring normalizedKey = Utility::Normalize(logicalKey);
		cache.erase(normalizedKey);
	}

	void Clear()           { cache.clear(); }
	void Reserve(size_t n) { cache.reserve(n); }


private:
	TRegistry& registry;
	LoaderFunc loadFunc;
	unordered_map<wstring, TResourcePtr> cache;
};

NS_END