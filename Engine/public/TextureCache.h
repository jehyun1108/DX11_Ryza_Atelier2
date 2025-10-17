#pragma once

NS_BEGIN(Engine)
class Texture;

class ENGINE_DLL TextureCache
{
public:
	explicit TextureCache(TextureRegistry& registry) : registry(registry) {}

	// �����ϸ� ��ȯ, ������ �ε��Ͽ� ������ ��ȯ
	shared_ptr<Texture> Ensure(const wstring& logicalKey);
	// �������� ��ȯ
	shared_ptr<Texture> Get(const wstring& logicalKey) const;

	// ĳ�ÿ��� ����
	void Erase(const wstring& logicalKey);

	void Clear() { cache.clear(); }
	void Reserve(size_t n) { cache.reserve(n); }

private:
	TextureRegistry& registry;
	unordered_map<wstring, shared_ptr<Texture>> cache;
};

NS_END