#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL UIAnimSystem
{
public:
	explicit UIAnimSystem(UIRegistry& uiRegistry) : uiRegistry(uiRegistry) {}

	void Tick(float dt);

	void PlaySlideOnce(const wstring& archetypeKey, float startX, float startY, float endX, float endY, float dur, UIEasing easing = UIEasing::EaseOutCubic);
	void Nudge(const wstring& archetypeKey, float dx, float dy, float dur, UIEasing easing = UIEasing::EaseOutCubic);

	void PlayFadeOnce(const wstring& archetypeKey, float fromOpacity, float toOpacity, float dur, UIEasing easing = UIEasing::EaseOutCubic);
	void PlayScaleOnce(const wstring& archetypeKey, float fromScaleX, float fromScaleY, float toScaleX, float toScaleY, float dur, UIEasing easing = UIEasing::EaseOutCubic);

private:
	UIRegistry& uiRegistry;
	unordered_map<wstring, UIAnimChannels> channels;
};

NS_END