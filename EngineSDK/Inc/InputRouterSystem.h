#pragma once

#include "InputRouterData.h"

NS_BEGIN(Engine)

class ENGINE_DLL InputRouterSystem
{
public:
	explicit InputRouterSystem(SystemRegistry& registry) : registry(registry) {}




private:
	SystemRegistry& registry;
};

NS_END