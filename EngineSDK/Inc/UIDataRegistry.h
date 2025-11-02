#pragma once

#include "UIData.h"

NS_BEGIN(Engine)

class ENGINE_DLL UIDataRegistry
{
public:
	explicit UIDataRegistry(AssetSystem& assets) : assets(assets) {}

	bool Register(const UITextureMeta& meta);
	void Upsert(const UITextureMeta& meta);
	const UITextureMeta* GetMeta(const wstring& texKey) const;
	shared_ptr<Texture> ResolveTexture(const wstring& texKey, const UITextureMeta** outMeta = nullptr) const;

	void Clear() { table.clear(); }

private:
	AssetSystem& assets;
	unordered_map<wstring, UITextureMeta> table;
};

NS_END