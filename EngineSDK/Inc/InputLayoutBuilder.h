#pragma once

#include "InputLayoutUtil.h"

NS_BEGIN(Engine)

class ENGINE_DLL InputLayoutBuilder
{
public:
	InputLayoutBuilder& Add(VtxAttribute attr, _uint semanticIdx = 0, _uint slot = 0, _uint stepRate = 0);
	InputLayoutBuilder& AddWorldMat(_uint slot = 1, _uint stepRate = 1);
	vector<D3D11_INPUT_ELEMENT_DESC> Build() const;

	void Clear();

	static InputLayoutBuilder MakePNUTan();
	static InputLayoutBuilder MakePNUTanSkin();
	static InputLayoutBuilder MakeInstancedPNUTan();
	static InputLayoutBuilder MakeVertexColor();

private:
	static void FillElement(VtxAttribute attr, _uint semanticIdx, _uint slot, _uint stepRate, D3D11_INPUT_ELEMENT_DESC& out);
	static _uint SizeOf(VtxAttribute attr);

private:
	static constexpr _uint maxSlots = 8;
	array<_uint, maxSlots> offsets = {};
	vector<D3D11_INPUT_ELEMENT_DESC> elements;
};

NS_END