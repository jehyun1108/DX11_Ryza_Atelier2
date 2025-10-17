#pragma once

NS_BEGIN(Engine)

enum class VtxAttribute : _uint
{
	None         = 0,
	Pos          = 1 << 0,
	Color        = 1 << 1,
	TexCoord     = 1 << 2,
	Normal       = 1 << 3,
	Tangent      = 1 << 4,
	World        = 1 << 5,
	BlendIndices = 1 << 6,
	BlendWeights = 1 << 7,
};

inline bool Has(VtxAttribute mask, VtxAttribute bit)
{
	return (static_cast<_uint>(mask) & static_cast<_uint>(bit)) != 0;
}

struct VtxElement
{
	VtxAttribute attribute = VtxAttribute::None;
	_uint        semanticIdx = {};
	_uint        slot = {};
	// 인스턴싱을 위한 설정, 0이면 per-vertex, 1 이상이면 per-instance.
	_uint        stepRate = {};
};

NS_END