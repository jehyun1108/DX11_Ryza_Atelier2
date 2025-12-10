#pragma once

NS_BEGIN(Engine)

enum class RENDERTYPE { NONBLEND, BLEND, END };
enum class RENDERPASS { Opaque, Outline, Transparent, Grid, Debug, END };
template<typename T, size_t N>
using StateArray = array<T, N>;

struct SortKey 
{
	uint64_t value = 0;
	static uint64_t Opaque(uint32_t materialId, uint32_t meshId) 	// 불투명: 20 Bit 씩
	{
		return (uint64_t(materialId & 0xFFFFF) << 44) |
			   (uint64_t(meshId     & 0xFFFFF) << 24);
	}
	// 투명: 거리 큰 것 번저 그리기 위해 거리 비트를 반전해서 오름차순 정렬로 내림차순 효과
	static uint64_t Transparent(float distance, uint32_t materialId, uint32_t meshId)
	{
		union { float t; uint32_t u; } convert{ distance };
		const uint32_t distBits = ~convert.u;
		return (uint64_t(distBits)             << 32) |
			   (uint64_t(materialId & 0xFFFFF) << 12) |
			   (uint64_t(meshId     & 0xFFFF));
	}
};
struct DrawItem // SortKey + Proxy
{
	SortKey     key{};
	RenderProxy proxy{};

	_uint cbFirst   = 0;
	_uint cbNum     = 0;
	_uint boneBase  = 0;
	_uint boneCount = 0;

	_uint layerMask = 0;
};
struct RenderQueues // Queue (Blend, NonBlend)
{
	vector<DrawItem> opaque;      // NONBLEND
	vector<DrawItem> transparent; // BLEND

	void Clear()
	{
		opaque.clear();
		transparent.clear();
	}
};
struct ParticleDrawItem  // Particle
{
	_float3 pos;
	float   size;
	_float4 color;
	float   rotRad;
	float   camDist;

	wstring texKey;
	_float2 uvMin{ 0.f, 0.f };
	_float2 uvMax{ 1.f, 1.f };
};
struct ParticleSnapshot
{
	vector<ParticleDrawItem> transparent;

	void Clear()
	{
		transparent.clear();
	}
};
struct TrailPointProxy
{
	_float3 pos;
	float   t;
};
struct TrailDrawItem
{
	vector<TrailPointProxy>   points;
	const TrailDesc*          desc;
	float                     camDist;
};
struct TrailSnapshot
{
	vector<TrailDrawItem> trails;
	void Clear() { trails.clear(); }
};
NS_END