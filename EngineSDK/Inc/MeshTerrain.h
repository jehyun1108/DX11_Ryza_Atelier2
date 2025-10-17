#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL TerrainMesh final : public StaticMesh
{
public:
	TerrainMesh() = default;
	virtual ~TerrainMesh() = default;

public:
	virtual HRESULT Load(const wstring& fullPath, const any& arg = {}) override;

private:
	_uint width{};
	_uint height{};
};

NS_END