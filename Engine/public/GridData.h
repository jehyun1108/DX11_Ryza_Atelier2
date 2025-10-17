#pragma once

NS_BEGIN(Engine)

struct GridCoord 
{
	int x = 0;
	int z = 0;
};

struct CellRect 
{
	int minX = 0;
	int maxX = 0;
	int minZ = 0;
	int maxZ = 0;
	
	bool Empty() const { return maxX < minX || maxZ < minZ; }
};

struct GridParams
{
	float   cellSize   = 1.f;
	_float3 origin     = {};
	int     cellCountX = 50;
	int     cellCountZ = 50;
	int     majorEvery = 5;

	_float4 colorMinor       = { 0.2f, 0.2f, 0.2f, 0.8f };
	_float4 colorMajor       = { 0.4f, 0.4f, 0.4f, 1.f }; 
	_float4 colorHover       = { 0.4f, 1.f, 0.4f, 0.45f };
	_float4 colorHoverBorder = { 0.2f, 1.f, 0.4f, 1.f };

	bool showMajor = true;
	bool showMinor = true;
	bool showHover = true;
};

struct GridData
{
	GridParams params{};
	bool       linesDirty  = true;
	bool       hoverDirty  = true;

	bool       hoverValid  = false;
	CellRect   hoverRect{};

	ComPtr<ID3D11Buffer> vbLines;
	ComPtr<ID3D11Buffer> vbHoverFill;
	ComPtr<ID3D11Buffer> vbHoverBorder;

	_uint vbLinesCapacityBytes       = 0;
	_uint vbHoverFillCapacityBytes   = 0;
	_uint vbHoverBorderCapacityBytes = 0;

	_uint lineVertexCount            = 0;
	_uint hoverFillVertexCount       = 0;
	_uint hoverBorderVertexCount     = 0;
};

NS_END