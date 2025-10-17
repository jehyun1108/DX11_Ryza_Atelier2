#pragma once

NS_BEGIN(Engine)

struct ENGINE_DLL LightData
{
	Handle     transform;
	LightProxy proxy{};
	bool       enabled = true;
};

NS_END