#pragma once

#include "LayerUtil.h"

NS_BEGIN(Engine)

struct ENGINE_DLL LayerData
{
	Handle   transform;
	_uint    layerMask = 0;
	bool     enabled = true;
};

NS_END