#pragma once

#include "ShaderUtil.h"

NS_BEGIN(Engine)

class ENGINE_DLL ShaderRegistry
{
public:
	void Register(const wstring& logicalKey, const ShaderMeta& meta);
	bool Exists(const wstring& logicalKey) const;
	
	const ShaderMeta* Get(const wstring& logicalKey) const;
	const ShaderMeta* GetByNormalizedKey(const wstring& normalizedKey) const;

	void Remove(const wstring& logicalKey);

	void Clear() { table.clear(); }

private:
	unordered_map<wstring, ShaderMeta> table;
};

NS_END