#pragma once

NS_BEGIN(Client)

class EffectLoader
{
public:
	static void LoadEffect(SystemRegistry& registry);

	static void LoadRyzaEffect(SystemRegistry& registry);
	static void LoadPatriciaEffect(SystemRegistry& registry);
	static void LoadKlaudiaEffect(SystemRegistry& registry);
};

NS_END