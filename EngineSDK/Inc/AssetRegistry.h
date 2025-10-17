#pragma once

NS_BEGIN(Engine)

template<typename TMeta>
class AssetRegistry
{
public:
	bool Register(const wstring& logicalKey, const TMeta& meta)
	{
		const wstring normalizedKey = Utility::Normalize(logicalKey);
		return table.emplace(normalizedKey, meta).second;
	}

	void Upsert(const wstring& logicalKey, const TMeta& meta)
	{
		const wstring normalizedKey = Utility::Normalize(logicalKey);
		table[normalizedKey] = meta;
	}

	bool Exists(const wstring& logicalKey) const
	{
		const wstring normalizedKey = Utility::Normalize(logicalKey);
		return table.find(normalizedKey) != table.end();
	}

	const TMeta* Get(const wstring& logicalKey) const
	{
		const wstring normalizedKey = Utility::Normalize(logicalKey);
		auto it = table.find(normalizedKey);
		return (it == table.end()) ? nullptr : &it->second;
	}

	const TMeta* GetByNormalizedKey(const wstring& normalizedKey) const
	{
		auto it = table.find(normalizedKey);
		return (it == table.end()) ? nullptr : &it->second;
	}

	bool Remove(const wstring& logicalKey)
	{
		const wstring normalizedKey = Utility::Normalize(logicalKey);
		return table.erase(normalizedKey) > 0;
	}

	void Clear() { table.clear(); }

private:
	unordered_map<wstring, TMeta> table;
};

NS_END