#pragma once

NS_BEGIN(Engine)

struct BattleStartParams
{
	BattleParty             allies;
	BattleEnemies           enemies;
	_float3                 centerWorld{};
	BattleSessionConfig     sessionConfig{};

	float startAngleDeg      = 0.f;
	float ringRadius         = 800.f;
	bool  faceCenterSnap     = true;
};

NS_END