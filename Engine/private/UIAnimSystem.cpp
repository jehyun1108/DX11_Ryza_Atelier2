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
	case UIEasing::EaseWave:
	{
		const float freq = 2.5f;
		const float damping = 3.0f;
		return 1.0f - expf(-damping * t) * cosf(2 * XM_PI * freq * t);
	}
	case UIEasing::EaseInOut:
	{
		if (t < 0.5f)
			return 4.f * t * t * t;
		else
		{
			const float u = -2.f * t + 2.f; 
			return 1.f - (u * u * u) * 0.5f;
		}
	}
	default: return t;
	}
}
static inline float MoveTowards(float cur, float target, float maxDelta)
{
	if (cur < target) return (cur + maxDelta < target) ? cur + maxDelta : target;
	if (cur > target) return (cur - maxDelta > target) ? cur - maxDelta : target;
	return target;
}
static inline void SampleShake(const UIShakeTrack& tr, float& outX, float& outY)
{
	const float e = expf(-tr.decay * tr.t);
	const float s = 6.2831853f * tr.freq * tr.t; 
	outX = tr.ampX * e * sinf(s + tr.phaseX);
	outY = tr.ampY * e * sinf(s + tr.phaseY);
}
// ---------------------------------------------------------------------------------------------------------------------

void UIAnimSystem::OnBoot()
{
	uiRegistry = &registry.Get<UIRegistry>();
}

void UIAnimSystem::Tick(float dt)
{
	unordered_set<wstring> keys;
	keys.reserve(channels.size() + spinSpeed.size() + spinAngleDeg.size());
	for (const auto& pair : channels)      keys.insert(pair.first);
	for (const auto& pair : spinSpeed)     keys.insert(pair.first);
	for (const auto& pair : spinAngleDeg)  keys.insert(pair.first);
	for (const auto& pair : shakeTracks)   keys.insert(pair.first);

	if (keys.empty()) return;

	auto advance = [&](UIAnimChannel& channel, float dtStep, float defaultValue) -> float
		{
			if (channel.dur < 0.f)
				return channel.end;

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

	for (const auto& key : keys)
	{
		UIAnimChannels& set  = channels[key];
		UIInstance&     inst = uiRegistry->Ensure(key);

		const float offX   = advance(set.offsetX, dt, 0.f);
		const float offY   = advance(set.offsetY, dt, 0.f);
		const float scaleX = advance(set.scaleX,  dt, 1.f);
		const float scaleY = advance(set.scaleY,  dt, 1.f);
		float       alpha  = clamp(advance(set.opacity, dt, 1.f), 0.f, 1.f);
		
		float rot = 0.f;
		if (spinAngleDeg.count(key))
			rot = spinAngleDeg[key];
		else
			rot = advance(set.rotDeg, dt, 0.f);

		if (auto it = spinSpeed.find(key); it != spinSpeed.end())
			rot += it->second * dt;

		if (rot >= 360.f || rot <= -360.f)
			rot = fmodf(rot, 360.f);
		if (rot < 0.f)
			rot += 360.f;
		spinAngleDeg[key] = rot;

		float shx = 0.f, shy = 0.f;
		if (auto it = shakeTracks.find(key); it != shakeTracks.end())
		{
			auto& tr = it->second;
			tr.t += dt;
			SampleShake(tr, shx, shy);
			if (tr.t >= tr.dur) 
			{ 
				shakeTracks.erase(it); 
				shx = shy = 0.f; 
			}
		}

		inst.animOffsetX = offX + shx;
		inst.animOffsetY = offY + shy;
		inst.animScaleX  = scaleX;
		inst.animScaleY  = scaleY;
		inst.animAlpha   = alpha;
		inst.animRotDeg  = rot;
	}

	for (auto& kv : fillChannels)
	{
		const std::wstring& key = kv.first;
		auto& ch = kv.second;
		if (!ch.active) continue;

		const float target  = ch.dst;
		const bool  rising  = (target > ch.cur);
		const float perSec  = rising ? ch.spec.upPerSec : ch.spec.downPerSec;
		const float step    = perSec * dt;

		ch.cur = MoveTowards(ch.cur, target, step);

		const float out = (ch.spec.origin == UIFillOrigin::Start) ? ch.cur : (1.f - ch.cur);
		if (ch.spec.axis == UIFillAxis::X) uiRegistry->SetFillRatioX(key, out);
		else                               uiRegistry->SetFillRatioY(key, out);
	}
}

void UIAnimSystem::PlaySlideOnce(const wstring& archetypeKey, float startX, float startY, float endX, float endY, float dur, UIEasing easing)
{
	uiRegistry->Ensure(archetypeKey);

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
	uiRegistry->Ensure(archetypeKey);

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
	uiRegistry->Ensure(archetypeKey);

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
	uiRegistry->Ensure(key);
	auto& channel          = channels[key];
	channel.scaleX.playing = false;
	channel.scaleX.elapsed = 0.f;
	channel.scaleX.dur     = -1.f;
	channel.scaleX.start   = scaleX;
	channel.scaleX.end     = scaleX;
	
	channel.scaleY.playing = false;
	channel.scaleY.elapsed = 0.f;
	channel.scaleY.dur     = -1.f;
	channel.scaleY.start   = scaleY;
	channel.scaleY.end     = scaleY;
}

void UIAnimSystem::SetOpacity(const wstring& key, float alpha)
{
	uiRegistry->Ensure(key);
	auto& channel           = channels[key];
	channel.opacity.playing = false;
	channel.opacity.elapsed = 0.f;
	channel.opacity.dur     = -1.f;
	channel.opacity.start   = alpha;
	channel.opacity.end     = alpha;
}

void UIAnimSystem::SetOffSet(const wstring& key, float offsetX, float offsetY)
{
	uiRegistry->Ensure(key);
	auto& channel           = channels[key];
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
	UIInstance& inst = uiRegistry->Ensure(key);
	auto& channel    = channels[key];

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
	UIInstance& inst = uiRegistry->Ensure(key);
	auto& ch = channels[key];
	float startA = inst.animAlpha;

	if (ch.opacity.dur < 0.f)
		startA = ch.opacity.end;

	ch.opacity.playing = true;
	ch.opacity.elapsed = 0.f;
	ch.opacity.dur     = max(0.f, dur);
	ch.opacity.start   = startA;
	ch.opacity.end     = toA;
	ch.opacity.easing  = easing;
}

void UIAnimSystem::OffsetTo(const wstring& key, float toX, float toY, float dur, UIEasing easing)
{
	UIInstance& inst = uiRegistry->Ensure(key);
	auto& ch = channels[key];

	ch.offsetX.playing = true;
	ch.offsetX.elapsed = 0.f;
	ch.offsetX.dur     = max(0.f, dur);
	ch.offsetX.start   = inst.animOffsetX;
	ch.offsetX.end     = toX;
	ch.offsetX.easing  = easing;

	ch.offsetY.playing = true;
	ch.offsetY.elapsed = 0.f;
	ch.offsetY.dur     = max(0.f, dur);
	ch.offsetY.start   = inst.animOffsetY;
	ch.offsetY.end     = toY;
	ch.offsetY.easing  = easing;
}

void UIAnimSystem::SetFill(const wstring& key, float fillX, float fillY)
{
	UIInstance& inst = uiRegistry->Ensure(key);
	inst.fillRatioX = fillX;
	inst.fillRatioY = fillY;
}

void UIAnimSystem::FillSetImmediate(const wstring& key, float ratio, const UIFillSpec& spec)
{
	uiRegistry->Ensure(key);
	auto& ch = fillChannels[key];
	ch.cur = ch.dst = Utility::Saturate(ratio);
	ch.spec = spec;
	ch.active = true;

	const float v = (spec.origin == UIFillOrigin::Start) ? ch.cur : (1.f - ch.cur);
	if (spec.axis == UIFillAxis::X) uiRegistry->SetFillRatioX(key, v);
	else                            uiRegistry->SetFillRatioY(key, v);
}

void UIAnimSystem::FillTo(const wstring& key, float targetRatio, const UIFillSpec& spec)
{
	uiRegistry->Ensure(key); 
	auto& ch = fillChannels[key];
	ch.dst = Utility::Saturate(targetRatio);
	ch.spec = spec;
	ch.active = true;
}

void UIAnimSystem::Spin(const wstring& key, float rotSpeed)
{
	uiRegistry->Ensure(key);
	spinSpeed[key] = rotSpeed;

	if (!spinAngleDeg.count(key))
		spinAngleDeg[key] = 0.f;
}

void UIAnimSystem::SetRotDeg(const wstring& key, float rotDeg)
{
	uiRegistry->Ensure(key);

	UIAnimChannels& ch = channels[key];

	ch.rotDeg.playing = false;
	ch.rotDeg.elapsed = 0.f;
	ch.rotDeg.dur     = -1.f;
	ch.rotDeg.start   = rotDeg;
	ch.rotDeg.end     = rotDeg;

	spinSpeed.erase(key);
	spinAngleDeg[key] = rotDeg;
}

void UIAnimSystem::PlayShakeOnce(const wstring& key, const UIShakeSpec& spec, float dirX, float scale)
{
	uiRegistry->Ensure(key);
	auto& tr = shakeTracks[key];

	if (tr.active)
	{
		tr.ampX += 0.5f * spec.ampX * scale * dirX;
		tr.ampY += 0.5f * spec.ampY * scale;
		tr.dur = max(tr.dur, spec.dur);
		return;
	}

	tr.active = true;
	tr.t      = 0.f;
	tr.ampX   = spec.ampX * scale * dirX;
	tr.ampY   = spec.ampY * scale;
	tr.freq   = spec.freq;
	tr.decay  = spec.decay;
	tr.dur    = spec.dur;
	tr.phaseX = spec.phaseX;
	tr.phaseY = spec.phaseY;
}