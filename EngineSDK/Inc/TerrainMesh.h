#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL TerrainMesh final : public StaticMesh
{
public:
	TerrainMesh() = default;
	virtual ~TerrainMesh() = default;

public:
	static shared_ptr<TerrainMesh> CreateFromHeightMap(const wstring& heightPath);

private:
	HRESULT Init(const wstring& heightPath);

private:
	_uint width{};
	_uint height{};
};

NS_END