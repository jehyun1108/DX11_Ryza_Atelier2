#pragma once

NS_BEGIN(Engine)

namespace LayerUtil
{
	// ���� ���̾� -> ���� ��Ʈ ����ũ
	inline constexpr uint32_t LayerBit(LAYER layer)
	{
		return 1u << ENUM(layer);
	}

	// ���� ���̾ OR�� ���� ���� ����
	template<typename... Layers>
	inline constexpr uint32_t Mask(Layers... layers)
	{
		return (LayerBit(layers) | ...);
	}

	// ����ũ�� ���̾ ���ԵǾ� �ִ���
	inline constexpr bool Has(uint32_t mask, LAYER layer)
	{
		return (mask & LayerBit(layer)) != 0;
	}

	// ����ũ �߰�/����
	inline constexpr uint32_t Add(uint32_t mask, LAYER layer)    { return mask | LayerBit(layer); }
	inline constexpr uint32_t Remove(uint32_t mask, LAYER layer) { return mask & ~LayerBit(layer); }
}

NS_END