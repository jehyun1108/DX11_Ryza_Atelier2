#pragma once

NS_BEGIN(Engine)

struct DamageFontSet
{
    wstring digits[10];
};
struct DamageHudSpec
{
    float baseX  = 0.f;
    float baseY  = 0.f;
    float gapX   = 28.f;
    float gapY   = 0.f;

    float inSlideDx  = 30.f;
    float inDur      = 0.40f;

    float holdDur    = 3.f;

    float outSlideDx = 30.f;
    float outDur     = 0.40f;

    UIEasing inEase = UIEasing::EaseOutCubic;
    UIEasing outEase = UIEasing::EaseOutCubic;
};
struct DamageNumber
{
	vector<wstring> keys;
	float x        = 0.f;
	float y        = 0.f;
	float lifeTime = 1.f;
	float elapsed  = 0.f;
	bool  active   = false;

    _uint  spawnId     = 0;
    int    curValue    = 0;      
    int    targetValue = 0;   
    float  countAcc    = 0.f;

    float  anchorX = 0.f;   
    float  anchorY = 0.f;
    float  advanceX = 0.f;
	
	void Reset() { active = false; elapsed = 0.f; keys.clear(); }
};

struct ChainHUD
{
    int              value = 0;
    bool             visible = false;
    vector<wstring>  keys;
    float            anchorX = 0.f;
    float            anchorY = 0.f;
    float            advanceX = 0.f;
};

NS_END