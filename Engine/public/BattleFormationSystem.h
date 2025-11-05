#pragma once

#include "BattleFormationData.h"

NS_BEGIN(Engine)

class ENGINE_DLL BattleFormationSystem : public ISystem
{
public:
	explicit BattleFormationSystem(SystemRegistry& registry) : registry(registry) {}

	void Init(const _float3 center, int allyCount, int enemyCount, const FormationParams& fParams);

	void SetCenter(const _float3& center)                 { centerWorld             = center;                                    dirty = true; }
	void SetRingRadius(float radius)                      { params.ringRadius       = max(1.f, radius);                          dirty = true; }
	void SetAllyStartDeg(float deg)                       { params.allyStartDeg     = deg;                                       dirty = true; }
	void SetEnemyStartDeg(float deg)                      { params.enemyStartDeg    = deg;                                       dirty = true; }
	void SetTeamSpanDeg(float allySpan, float enemySpan)  { params.allySpanDeg      = allySpan; params.enemySpanDeg = enemySpan; dirty = true; }
	void SetCharRadius(float meter)                       { params.charRadiusMeters = max(0.f, meter);                           dirty = true; }
	void SetPadDeg(float deg)                             { params.padDeg           = max(0.f, deg);                             dirty = true; }

	void  Tick(float dt)             { if (dirty) ReCompute(); }
	float GetEffectiveRadius() const { return effectiveRadius; }

	bool           TryGetTargetPos(BattleTeam team, int slotIdx, _float3& out) const;
	bool           TryGetFaceDir(BattleTeam team, int slotIdx, _float2& out) const;
	const _float3& GetCenter() const { return centerWorld; }

private:
	void  ReCompute();

	static inline float DegToRad(float d) { return d * (XM_PI / 180.f); }
	static inline float RadToDeg(float r) { return r * (180.f / XM_PI); }

	static inline float RequiredRadiusForGap(float charR, float degGap, float padDeg);
	static inline float TeamRequiredRadius(int count, float spanDeg, float charR, float padDeg);
	static inline vector<float> MakeAngles(int n, float centerDeg, float spanDeg);

private:
	SystemRegistry& registry;

	_float3 centerWorld{};
	int allyCount  = 0;
	int enemyCount = 0;

	FormationParams params{};
	float effectiveRadius = 0.f;

	array<_float3, 3> allyTarget{};
	array<_float3, 3> enemyTarget{};
	array<_float2, 3> allyFace{};
	array<_float2, 3> enemyFace{};

	bool dirty = true;
};

NS_END