#include "Enginepch.h"

inline static float ApplyEasing(UIEasing easing, float t01)
{
	const float t = Utility::Saturate(t01);
	switch (easing)
	{
	case UIEasing::Linear:
		return t;
	case UIEasing::EaseOutQuad:
	{
		const float inv = 1.f - t;
		return 1.f - inv * inv;
	}
	case UIEasing::EaseOutCubic:
	{
		const float inv = 1.f - t;
		return 1.f - inv * inv * inv;
	}
	}
}
// ---------------------------------------------------------------------------------------------------------------------

void UIAnimSystem::Tick(float dt)
{
	if (channels.empty()) return;

	for (auto& pair : channels)
	{
		const wstring& archetypeKey = pair.first;
		UIAnimChannels& channelSet = pair.second;

		UIInstance& instance = uiRegistry.Ensure(archetypeKey);

		float resolvedOffsetX = 0.f;
		float resolvedOffsetY = 0.f;

		if (channelSet.offsetX.playing)
		{
			channelSet.offsetX.elapsed += dt;
			const float tNormalized = channelSet.offsetX.dur > 0.f ? (channelSet.offsetX.elapsed / channelSet.offsetX.dur) : 1.f;
			const float eased = ApplyEasing(channelSet.offsetX.easing, tNormalized);
			resolvedOffsetX = lerp(channelSet.offsetX.start, channelSet.offsetX.end, eased);

			if (tNormalized >= 1.f)
			{
				channelSet.offsetX.playing = false;
				resolvedOffsetX = channelSet.offsetX.end;
			}
		}
		else
			resolvedOffsetX = channelSet.offsetX.end;

		if (channelSet.offsetY.playing)
		{
			channelSet.offsetY.elapsed += dt;
			const float tNorm = channelSet.offsetY.dur > 0.f ? (channelSet.offsetY.elapsed / channelSet.offsetY.dur) : 1.f;
			const float eased = ApplyEasing(channelSet.offsetY.easing, tNorm);
			resolvedOffsetY = lerp(channelSet.offsetY.start, channelSet.offsetY.end, eased);

			if (tNorm >= 1.f)
			{
				channelSet.offsetY.playing = false;
				resolvedOffsetY = channelSet.offsetY.end;
			}
		}
		else
			resolvedOffsetY = channelSet.offsetY.end;

		instance.animOffsetX = resolvedOffsetX;
		instance.animOffsetY = resolvedOffsetY;
	}
}

void UIAnimSystem::PlaySlideOnce(const wstring& archetypeKey, float startX, float startY, float endX, float endY, float dur, UIEasing easing)
{
	(void)uiRegistry.Ensure(archetypeKey);

	UIAnimChannels& channel = channels[archetypeKey];

	channel.offsetX.playing = true;
	channel.offsetX.elapsed = 0.f;
	channel.offsetX.dur     = max(0.f, dur);
	channel.offsetX.start   = startX;
	channel.offsetX.end     = endX;
	channel.offsetX.easing  = easing;

	channel.offsetY.playing = true;
	channel.offsetY.elapsed = 0.f;
	channel.offsetY.dur     = max(0.f, dur);
	channel.offsetY.start   = startY;
	channel.offsetY.end     = endY;
	channel.offsetY.easing  = easing;
}

void UIAnimSystem::Nudge(const wstring& archetypeKey, float dx, float dy, float dur, UIEasing easing)
{
	PlaySlideOnce(archetypeKey, dx, dy, 0.f, 0.f, dur, easing);
}

void UIAnimSystem::PlayFadeOnce(const wstring& archetypeKey, float fromOpacity, float toOpacity, float dur, UIEasing easing)
{
	(void)uiRegistry.Ensure(archetypeKey);

	UIAnimChannels& channel = channels[archetypeKey];

	channel.opacity.playing = true;
	channel.opacity.elapsed = 0.f;
	channel.opacity.dur     = max(0.f, dur);
	channel.opacity.start   = fromOpacity;   // ∫∏≈Î 0 °Ê 1 or 1 °Ê 0
	channel.opacity.end     = toOpacity;
	channel.opacity.easing  = easing;
}

void UIAnimSystem::PlayScaleOnce(const wstring& archetypeKey, float fromScaleX, float fromScaleY, float toScaleX, float toScaleY, float dur, UIEasing easing)
{
	(void)uiRegistry.Ensure(archetypeKey);

	UIAnimChannels& channel = channels[archetypeKey];

	channel.scaleX.playing = true;
	channel.scaleX.elapsed = 0.f;
	channel.scaleX.dur     = max(0.f, dur);
	channel.scaleX.start   = fromScaleX;
	channel.scaleX.end     = toScaleX;
	channel.scaleX.easing  = easing;

	channel.scaleY.playing = true;
	channel.scaleY.elapsed = 0.f;
	channel.scaleY.dur     = max(0.f, dur);
	channel.scaleY.start   = fromScaleY;
	channel.scaleY.end     = toScaleY;
	channel.scaleY.easing  = easing;
}