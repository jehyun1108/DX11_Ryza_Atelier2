#pragma once

#include "UIBatchData.h"

NS_BEGIN(Engine)

// 배치 수집기: 같은 텍스처끼리 모아 드로우콜 최소화
class ENGINE_DLL UIDrawBatch
{
public:
	void Push(const UIDrawImage& image);
	void BuildAndRender(ID3D11DeviceContext* context);

	void Clear() { items.clear(); }

	vector<UIDrawItem> DetachItems();

private:
	vector<UIDrawItem> items;
};

NS_END