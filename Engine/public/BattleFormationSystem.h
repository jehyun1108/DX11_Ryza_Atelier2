#pragma once

#include "BattleFormationData.h"

NS_BEGIN(Engine)

class ENGINE_DLL BattleFormationSystem : public ISystem
{
public: 
	explicit BattleFormationSystem(SystemRegistry& registry) : registry(registry) {}

	void Init(const _float3 center, int allyCount, int enemyCount, const FormationParams& introParam, const FormationParams& battleParams);

	void SetPhase(FormationPhase newPhase);
	FormationPhase GetPhase() const { return phase; }

	void SetParams(FormationPhase which, const FormationParams& params);
	void SetCenter(const _float3& center);

	void  Tick(float dt)             { if (dirty) ReCompute(); }
	float GetEffectiveRadius() const { return effectiveRadius; }

	_float3        GetTargetPos(BattleTeam team, int slotIdx) const;
	_float2        GetFaceDir(BattleTeam team, int slotIdx) const;
	const _float3& GetCenter() const { return centerWorld; }

private:
	void ReCompute();
	void ReComputeIntro(const FormationParams& p, float baseR, int leaderSlot, const vector<float>& allyAngles, const vector<float>& enemyAngles);
	void ReComputeBattle(const FormationParams& p, float baseR, int leaderSlot, const vector<float>& allyAngles, const vector<float>& enemyAngles);
	void ComputeFacing();

	static float DegToRad(float d) { return d * (XM_PI / 180.f); }
	static float RadToDeg(float r) { return r * (180.f / XM_PI); }

	static float RequiredRadiusForGap(float charR, float degGap, float padDeg);
	static float TeamRequiredRadius(int count, float spanDeg, float charR, float padDeg);
	static vector<float> MakeAngles(int n, float centerDeg, float spanDeg);
	static float LengthXZ(const _float3& v);

private:
	_float3 centerWorld{};
	int     allyCount = 0;
	int     enemyCount = 0;
	float   effectiveRadius = 0.f;
	bool    dirty = true;

	array<_float3, 3> allyTarget{};
	array<_float3, 3> enemyTarget{};
	array<_float2, 3> allyFace{};
	array<_float2, 3> enemyFace{};

	FormationParams introParams{};
	FormationParams battleParams{};
	FormationPhase  phase = FormationPhase::Intro;

private:
	SystemRegistry& registry;
};

NS_END