#pragma once

NS_BEGIN(Engine)

struct UIAnimChannel
{
	bool     playing = false;
	float    elapsed = 0.f;
	float    dur     = 0.f;
	float    start   = 0.f;
	float    end     = 0.f;
	UIEasing easing  = UIEasing::EaseOutCubic;
};

struct UIAnimChannels
{
	UIAnimChannel offsetX, offsetY;
	UIAnimChannel scaleX, scaleY;
	UIAnimChannel opacity;
};

NS_END