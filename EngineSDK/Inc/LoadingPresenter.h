#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL LoadingPresenter : public ISystem
{
public:
	explicit LoadingPresenter(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void Enter();
	void Tick(float dt);
	void Exit();

	void OnLoadingComplete();
    bool IsFadeOutFinished() const { return state == State::Hidden; }

private:
    void InitDotsIfNeeded();
    void UpdateDots(float dt);
    void ApplyDotVisibility();

private:
    enum class State
    {
        Hidden, FadingIn, Showing, FadingOut
    };

    wstring bgKey      = L"loading_bg";
    wstring textKey    = L"now_loading";
    wstring dotBaseKey = L"loading_dot";

    array<wstring, 3> dotKeys{
        L"loading_dot_0",
        L"loading_dot_1",
        L"loading_dot_2"
    };

    State state           = State::Hidden;
    float stateTime       = 0.f;
    float fadeInDur       = 0.5f;
    float fadeOutDur      = 0.5f;
    float dotInterval     = 0.3f; 
    float dotTimer        = 0.f;
    int   visibleDotCount = 0;
    bool  loadingFinished = false;
    bool  dotsInitialized = false;

private:
	SystemRegistry& registry;
	UIRegistry*     uiRegistry{};
	UIAnimSystem*   uiAnimSys{};
};

NS_END