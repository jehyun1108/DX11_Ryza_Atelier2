#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL ScreenFadeSystem : public ISystem
{
public:
	explicit ScreenFadeSystem(SystemRegistry& registry) : registry(registry) {}

	void OnBoot() override;
	void Init();
	void Tick(float dt);

	void FadeIn(float dur = 0.5f);
	void FadeOut(float dur = 0.5f);

	void FadeInWhite(float dur = 0.5f);
	void FadeOutWhite(float dur = 0.5f);

	bool IsBusy()       const { return active; }
	bool IsFullyBlack() const { return curAlpha >= 0.999f && mode == FadeMode::Black; }
	bool IsFullyWhite() const { return curAlpha >= 0.999f; }

private:
	void SetMode(FadeMode newMode);
	void BeginFade(float target, float dur);

private:
	UIInstance* fadeBlack{};
	UIInstance* fadeWhite{};
	UIInstance* activeInst{};

	FadeMode mode = FadeMode::Black;

	float curAlpha    = 1.f;
	float startAlpha  = 1.f;
	float targetAlpha = 1.f;
	float timer       = 0.f;
	float dur         = 0.f;
	bool  active      = false;

private:
	SystemRegistry& registry;
	UIRegistry*     uiRegistry{};
};

NS_END