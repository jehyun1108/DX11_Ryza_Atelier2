#pragma once

NS_BEGIN(Engine)

enum class FormationPhase
{
	Intro, Battle,
};

struct FormationParams
{
	float ringRadius       = 500.f;
	float allyStartDeg     = 0.f;
	float enemyStartDeg    = 180.f;
	float allySpanDeg      = 120.f;
	float enemySpanDeg     = 120.f;
	float charRadiusMeters = 40.f;
	float padDeg           = 4.f;
	float backMeters       = 200.f;
	int   allyLeaderSlot   = -1;
};

NS_END