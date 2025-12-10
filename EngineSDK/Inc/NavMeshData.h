#pragma once

NS_BEGIN(Engine)

struct NavVertex
{
	_float3 posWorld;
};
struct NavTriangle
{
	_uint a, b, c;
};
struct NavTempBuffer
{
	_uint   count = 0;
	_float3 p[3];
	_uint   snappedVid[3]      = {UINT32_MAX, UINT32_MAX , UINT32_MAX };
	bool    snappedExisting[3] = { false, false, false };
};
struct NavStore 
{
	vector<NavVertex>   vertices;
	vector<NavTriangle> triangles;
};
struct NavigationData 
{
	NavStore       store;
	NavTempBuffer  temp;
};

NS_END