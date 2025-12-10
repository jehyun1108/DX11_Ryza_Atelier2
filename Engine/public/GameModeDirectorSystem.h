#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL GameModeDirectorSystem : public ISystem
{
public:
	explicit GameModeDirectorSystem(SystemRegistry& registry);
	void     OnBoot() override;

	void     Update(float dt);
	void     RequestSwitch(GameMode nextMode);
	bool     BeginBattle(const BattleStartParams& startParams);
	
	GameMode GetMode() const { return curMode; }
	void     Start()         { fieldSys->Enter(); }

private:
	GameMode               curMode;

private:
	SystemRegistry&         registry;
	UISystem*               uiSys{};
	FieldOrchestraSystem*   fieldSys{};
	BattleOrchestraSystem*  battleSys{};
	LogoOrchestraSystem*    logoSys{};
	ScreenFadeSystem*       fadeSys{};
	UIAnimSystem*           uiAnimSys{};
	ScreenDistortionSystem* distortSys{};
	SoundSystem*            soundSys{};
};

NS_END