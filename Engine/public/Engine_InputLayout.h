#pragma once

namespace Engine
{
	struct VertexColor
	{
		_float3 pos;
		_float4 color;
	};

	struct Vertex_PUV
	{
		_float3 pos;
		_float2 uv;
	};

	struct Vertex_PNU
	{
		_float3 pos;
		_float3 normal;
		_float2 uv;
	};

	struct Vertex_PNUTAN
	{
		_float3 pos;
		_float3 normal;
		_float2 uv;
		_float4 tangent;
	};

	struct Vertex_PNUTanSkin
	{
		_float3 pos;
		_float3 normal;
		_float2 uv;
		_float4 tangent;

		_uint  boneIndices[4]{};
		_float boneWeights[4]{};
	};
}