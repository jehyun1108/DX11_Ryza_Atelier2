#pragma once

NS_BEGIN(Engine)

enum class RewardState
{
	Idle, FadeInToStep, PlayingStep
};

class ENGINE_DLL BattleRewardPresenter : public ISystem
{
public:
	explicit BattleRewardPresenter(SystemRegistry& registry) : registry(registry) {}

	void OnBoot() override;
	void BeginVictory(const vector<EntityID>& order);
	void Tick(float dt);

	float CalcBattleDt(float dt);

	bool IsActive()   const { return active; }
	bool IsFinished() const { return finished; }

private:
	void  StartStep(int idx);
	void  UpdateCeremony(float dt);

	void  PlayClip(EntityID entity, AnimKey key, float crossFadeDur);
	float ResolveClipDur(EntityID entity, AnimKey key) const;

	void ShowResultUI();

private:
	bool            resultUiShown = false;
	struct Step
	{
		EntityID entity   = 0u;
		int      phase    = 0;   
		float    timer    = 0.f;
		float    durA     = 0.f;
		float    durB     = 0.f;
		float    gapAfter = 0.f;
	};

	vector<Step> steps;
	int          curIndex = -1;
	bool         active = false;
	bool         finished = false;

	float slowTimeRemaining = 0.f;
	float slowDur           = 0.f;
	float slowFactor        = 0.f;

	RewardState    state = RewardState::Idle;
	CamFadeProfile rewardFade{};

private:
	SystemRegistry&      registry;
	AnimatorSystem*      animator{};
	AnimDataSystem*      animSys{};
	CharacterDataSystem* dataSys{};
	ScreenFadeSystem*    fadeSys{};
	CamRegistry*         camReg{};
	SoundSystem*         soundSys{};
	UISystem*            uiSys{};
	UIRegistry*          uiRegistry{};
	UIAnimSystem*        uiAnimSys{};
};

NS_END