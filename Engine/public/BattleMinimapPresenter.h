#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL BattleMinimapPresenter : public ISystem
{
public:
	explicit BattleMinimapPresenter(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void     Enter();
	void     Tick(float dt);
	void     Exit();

private:
	SystemRegistry& registry;
};

NS_END