#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL HFSM_Actions
{
public:
	static void PauseAbsolute(SystemRegistry& registry, Handle animHandle, _uint layerIdx, bool shouldPause);

	static ActionRegistry BuildDefaultActions(SystemRegistry& registry);
};

NS_END