#pragma once

NS_BEGIN(Engine)

struct HPState
{
	int cur = 0;
	int max = 0;
};
struct StunState
{
	float cur  = 0.f;
	float max  = 100.f;
	bool  full = false;
};
struct BattleAttributeConfig
{
	float stunMax         = 100.f;
	float stunOnSkillHit  = 10.f;
	float stunDecayPerSec = 0.f; 
	bool  clearOnFull     = false;
	float stunPerDamage   = 0.25f;
};

NS_END