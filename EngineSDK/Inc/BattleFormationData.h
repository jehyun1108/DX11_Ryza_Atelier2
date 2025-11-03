#pragma once

NS_BEGIN(Engine)

struct FormationParams
{
	float ringRadius       = 800.f;
	float allyStartDeg     = 0.f;
	float enemyStartDeg    = 180.f;
	float allySpanDeg      = 120.f;
	float enemySpanDeg     = 120.f;
	float charRadiusMeters = 40.f;
	float padDeg           = 4.f;
};

NS_END