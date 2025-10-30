#pragma once

NS_BEGIN(Engine)

class InputRoutingSystem
{
public:
	explicit InputRoutingSystem(SystemRegistry& registry) : registry(registry) {}

	void Update(float dt);

private:
	SystemRegistry& registry;
};

NS_END