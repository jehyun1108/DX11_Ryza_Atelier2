#pragma once

#include "UIAnimData.h"

NS_BEGIN(Engine)

class ENGINE_DLL UIAnimSystem : public ISystem
{
public:
	explicit UIAnimSystem(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;
	void     Tick(float dt);

	void PlaySlideOnce(const wstring& archetypeKey, float startX, float startY, float endX, float endY, float dur, UIEasing easing = UIEasing::EaseInOut);
	void Nudge(const wstring& archetypeKey, float dx, float dy, float dur, UIEasing easing = UIEasing::EaseInOut);

	void PlayFadeOnce(const wstring& archetypeKey, float fromOpacity, float toOpacity, float dur, UIEasing easing = UIEasing::EaseInOut);
	void PlayScaleOnce(const wstring& archetypeKey, float fromScaleX, float fromScaleY, float toScaleX, float toScaleY, float dur, UIEasing easing = UIEasing::EaseInOut);

	void SetScale(const wstring& key, float scaleX, float scaleY);
	void SetOpacity(const wstring& key, float alpha);
	void SetOffSet(const wstring& key, float offsetX, float offsetY);

	void ScaleTo(const wstring& key, float toX, float toY, float dur, UIEasing easing = UIEasing::EaseInOut);
	void FadeTo(const wstring& key, float toA, float dur, UIEasing easing = UIEasing::EaseInOut);
	void OffsetTo(const wstring& key, float toX, float toY, float dur, UIEasing easing = UIEasing::EaseInOut);

	void SetFill(const wstring& key, float fillX, float fillY);
	void FillSetImmediate(const wstring& key, float ratio, const UIFillSpec& spec);
	void FillTo(const wstring& key, float targetRatio, const UIFillSpec& spec);
	
	void PlayShakeOnce(const wstring& key, const UIShakeSpec& spec, float dirX = 1.f, float scale = 1.f);
	void Spin(const wstring& key, float rotSpeed);

	void SetRotDeg(const wstring& key, float rotDeg);
	void FillStop(const wstring& key) { fillChannels.erase(key); }
	void StopSpin(const wstring& key) { spinSpeed.erase(key); }
	void StopShake(const wstring& key) { shakeTracks.erase(key); }

private:
	unordered_map<wstring, UIAnimChannels> channels;
	unordered_map<wstring, float>          spinSpeed;
	unordered_map<wstring, float>          spinAngleDeg;
	unordered_map<wstring, UIFillChannel>  fillChannels;
	unordered_map<wstring, UIShakeTrack>   shakeTracks;

private:
	SystemRegistry& registry;
	UIRegistry*     uiRegistry{};
};

NS_END