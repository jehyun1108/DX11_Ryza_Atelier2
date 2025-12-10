#pragma once

NS_BEGIN(Engine)

enum class UIFillAxis { X, Y };
enum class UIFillOrigin { Start, End };
struct UIFillSpec
{
	UIFillAxis   axis       = UIFillAxis::X;
	UIFillOrigin origin     = UIFillOrigin::Start;
	float        upPerSec   = 3.f; 
	float        downPerSec = 1.2f;
};
struct UIFillChannel
{
	float cur = 0.f;   
	float dst = 0.f;   
	UIFillSpec  spec{};
	bool  active = false;
};
struct UIAnimChannel
{
	bool     playing = false;
	float    elapsed = 0.f;
	float    dur     = 0.f;
	float    start   = 0.f;
	float    end     = 0.f;
	UIEasing easing  = UIEasing::EaseInOut;
};
struct UIAnimChannels
{
	UIAnimChannel offsetX, offsetY;
	UIAnimChannel scaleX,  scaleY;
	UIAnimChannel opacity;
	UIAnimChannel rotDeg;
};
struct UIShakeSpec
{
	float ampX   = 28.f;  
	float ampY   = 12.f;
	float freq   = 14.f;  
	float decay  = 6.f;  
	float dur    = 0.4f;  
	float phaseX = 0.f;    
	float phaseY = 1.57f;
};
struct UIShakeTrack
{
	bool  active = false;
	float t = 0.f;
	float ampX = 0.f;
	float ampY = 0.f;
	float freq = 12.f;
	float decay = 6.f;
	float dur = 0.4f;
	float phaseX = 0.f;
	float phaseY = 1.57f;
};

NS_END