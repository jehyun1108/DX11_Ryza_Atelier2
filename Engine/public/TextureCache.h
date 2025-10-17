#pragma once

NS_BEGIN(Engine)
class Texture;

class ENGINE_DLL TextureCache
{
public:
	explicit TextureCache(TextureRegistry& registry) : registry(registry) {}

	// 존재하면 반환, 없으면 로드하여 보관후 반환
	shared_ptr<Texture> Ensure(const wstring& logicalKey);
	// 있을때만 반환
	shared_ptr<Texture> Get(const wstring& logicalKey) const;

	// 캐시에서 제거
	void Erase(const wstring& logicalKey);

	void Clear() { cache.clear(); }
	void Reserve(size_t n) { cache.reserve(n); }

private:
	TextureRegistry& registry;
	unordered_map<wstring, shared_ptr<Texture>> cache;
};

NS_END