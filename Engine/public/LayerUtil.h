#pragma once

NS_BEGIN(Engine)

namespace LayerUtil
{
	// 단일 레이어 -> 단일 비트 마스크
	inline constexpr uint32_t LayerBit(LAYER layer)
	{
		return 1u << ENUM(layer);
	}

	// 여러 레이어를 OR로 묶기 편한 버전
	template<typename... Layers>
	inline constexpr uint32_t Mask(Layers... layers)
	{
		return (LayerBit(layers) | ...);
	}

	// 마스크에 레이어가 포함되어 있는지
	inline constexpr bool Has(uint32_t mask, LAYER layer)
	{
		return (mask & LayerBit(layer)) != 0;
	}

	// 마스크 추가/제거
	inline constexpr uint32_t Add(uint32_t mask, LAYER layer)    { return mask | LayerBit(layer); }
	inline constexpr uint32_t Remove(uint32_t mask, LAYER layer) { return mask & ~LayerBit(layer); }
}

NS_END