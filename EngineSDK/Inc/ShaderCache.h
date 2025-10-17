#pragma once

NS_BEGIN(Engine)
class Shader;

class ENGINE_DLL ShaderCache
{
public:
	explicit ShaderCache(ShaderRegistry& registry) : registry(registry) {}

	shared_ptr<Shader> Ensure(const wstring& logicalKey);
	shared_ptr<Shader> Get(const wstring& logicalKey) const;

	void Erase(const wstring& logicalKey);

	void Clear() { cache.clear(); }
	void Reserve(size_t n) { cache.reserve(n); }

private:
	ShaderRegistry& registry;
	unordered_map<wstring, shared_ptr<Shader>> cache;
};

NS_END