#pragma once

NS_BEGIN(Engine)

enum class RENDERTYPE { NONBLEND, BLEND, END };

// 정렬
struct SortKey
{
	uint64_t value = 0;

	// 불투명: 20 Bit 씩
	static uint64_t Opaque(uint32_t materialId, uint32_t meshId)
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

// SortKey + Proxy
struct DrawItem
{
	SortKey     key{};
	RenderProxy proxy{};
};

// Queue (Blend, NonBlend)
struct RenderQueues
{
	vector<DrawItem> opaque;      // NONBLEND
	vector<DrawItem> transparent; // BLEND

	void Clear()
	{
		opaque.clear();
		transparent.clear();
	}
};

NS_END