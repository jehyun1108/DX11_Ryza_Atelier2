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
	return {};
	}
}
// ---------------------------------------------------------------------------------------------------------------------

void UIAnimSystem::Tick(float dt)
{
	if (channels.empty()) return;

	auto advance = [&](UIAnimChannel& channel, float dtStep, float defaultValue) -> float
		{
			if (channel.playing)
			{
				channel.elapsed   += dtStep;
				const float tNorm  = (channel.dur > 0.f) ? (channel.elapsed / channel.dur) : 1.f;
				const float tClamp = clamp(tNorm, 0.f, 1.f);
				const float eased  = ApplyEasing(channel.easing, tClamp);
				float       value  = lerp(channel.start, channel.end, eased);

				if (tNorm >= 1.f)
				{
					channel.playing = false;
					channel.elapsed = channel.dur;
					value           = channel.end;
				}
				return value;
			}
			const bool untouched = (channel.elapsed == 0.f && channel.dur == 0.f && channel.start == 0.f && channel.end == 0.f);
			return untouched ? defaultValue : channel.end;
		};

	for (auto& pair : channels)
	{
		const wstring&  key  = pair.first;
		UIAnimChannels& set  = pair.second;
		UIInstance&     inst = uiRegistry.Ensure(key);

		const float offX   = advance(set.offsetX, dt, 0.f);
		const float offY   = advance(set.offsetY, dt, 0.f);
		const float scaleX = advance(set.scaleX,  dt, 1.f);
		const float scaleY = advance(set.scaleY,  dt, 1.f);
		float opacity      = advance(set.opacity, dt, 1.f);
		opacity            = clamp(opacity, 0.f, 1.f);

		inst.animOffsetX = offX;
		inst.animOffsetY = offY;
		inst.animScaleX = scaleX;
		inst.animScaleY = scaleY;
		inst.animOpacity = opacity;
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
	channel.opacity.start   = fromOpacity;   
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

void UIAnimSystem::SetScale(const wstring& key, float scaleX, float scaleY)
{
	uiRegistry.Ensure(key);
	auto& channel = channels[key];
	channel.scaleX.playing = false;
	channel.scaleX.elapsed = 0.f;
	channel.scaleX.dur     = 0.f;
	channel.scaleX.start   = scaleX;
	channel.scaleX.end     = scaleX;
	
	channel.scaleY.playing = false;
	channel.scaleY.elapsed = 0.f;
	channel.scaleY.dur     = 0.f;
	channel.scaleY.start   = scaleY;
	channel.scaleY.end     = scaleY;
}

void UIAnimSystem::SetOpacity(const wstring& key, float alpha)
{
	uiRegistry.Ensure(key);
	auto& channel = channels[key];
	channel.opacity.playing = false;
	channel.opacity.elapsed = 0.f;
	channel.opacity.dur     = 0.f;
	channel.opacity.start   = alpha;
	channel.opacity.end     = alpha;
}

void UIAnimSystem::SetOffSet(const wstring& key, float offsetX, float offsetY)
{
	uiRegistry.Ensure(key);
	auto& channel = channels[key];
	channel.offsetX.playing = false;
	channel.offsetX.elapsed = 0.f;
	channel.offsetX.dur     = 0.f;
	channel.offsetX.start   = offsetX;
	channel.offsetX.end     = offsetX;

	channel.offsetY.playing = false;
	channel.offsetY.elapsed = 0.f;
	channel.offsetY.dur     = 0.f;
	channel.offsetY.start   = offsetY;
	channel.offsetY.end     = offsetY;
}

void UIAnimSystem::ScaleTo(const wstring& key, float toX, float toY, float dur, UIEasing easing)
{
	UIInstance& inst = uiRegistry.Ensure(key);
	auto& channel = channels[key];

	channel.scaleX.playing = true;
	channel.scaleX.elapsed = 0.f;
	channel.scaleX.dur     = max(0.f, dur);
	channel.scaleX.start   = inst.animScaleX;
	channel.scaleX.end     = toX;
	channel.scaleX.easing  = easing;

	channel.scaleY.playing = true;
	channel.scaleY.elapsed = 0.f;
	channel.scaleY.dur     = max(0.f, dur);
	channel.scaleY.start   = inst.animScaleY;
	channel.scaleY.end     = toY;
	channel.scaleY.easing  = easing;
}

void UIAnimSystem::FadeTo(const wstring& key, float toA, float dur, UIEasing easing)
{

}

void UIAnimSystem::OffsetTo(const wstring& key, float toX, float toY, float dur, UIEasing easing)
{

}
