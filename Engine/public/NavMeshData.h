#pragma once

NS_BEGIN(Engine)

struct NavMeshComponent
{
	vector<_float3> localPos{};
	vector<_uint>   indices;
};

struct BoundaryEdge
{
	_uint start;
	_uint end;
};

struct NavMeshRenderParams
{
    _float4 colorSolid    = { 0.20f, 0.80f, 0.40f, 0.35f }; 
    _float4 colorWire     = { 0.05f, 0.05f, 0.05f, 1.00f }; 
    _float4 colorBoundary = { 1.00f, 0.60f, 0.10f, 1.00f }; 

    bool  showSolid    = true;
    bool  showWire     = true;
    bool  showBoundary = true;

    float epsilon = 0.001f; // Z-fighting ¹æÁö
};

NS_END