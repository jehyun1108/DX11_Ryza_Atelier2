#include "Enginepch.h"

static inline _float3 PosOnRing(const _float3& center, float deg, float radius)
{
	const float rad = XMConvertToRadians(deg);
	return _float3{ center.x + sinf(rad) * radius, center.y, center.z + cosf(rad) * radius };
}

inline float BattleFormationSystem::RequiredRadiusForGap(float charR, float degGap, float padDeg)
{
    const float eff     = max(1e-3f, degGap - padDeg);
    const float halfRad = DegToRad(eff) * 0.5f;
    const float s       = sinf(halfRad);
    if (s <= 1e-5f)  return FLT_MAX;
    return charR / s;
}

inline float BattleFormationSystem::TeamRequiredRadius(int count, float spanDeg, float charR, float padDeg)
{
    if (count <= 1) return 0.f;
    const float degGap = spanDeg / float(max(1, count - 1));
    return RequiredRadiusForGap(charR, degGap, padDeg);
}

inline vector<float> BattleFormationSystem::MakeAngles(int n, float centerDeg, float spanDeg)
{
    vector<float> out; out.reserve(max(0, n));
    if (n <= 0) return out;
    if (n == 1) { out.push_back(centerDeg); return out; }
    const float start = centerDeg - spanDeg * 0.5f;
    const float step = spanDeg / float(n - 1);
    for (int i = 0; i < n; ++i) out.push_back(start + step * i);
    return out;
}
float BattleFormationSystem::LengthXZ(const _float3& v)
{
    return sqrtf(v.x * v.x + v.z * v.z);
}
// ---------------------------------------------------------------------------------------------------------------------------------
_float3 BattleFormationSystem::GetTargetPos(BattleTeam team, int slotIdx) const
{
    switch (team)
    {
    case BattleTeam::Ally:  return allyTarget[(size_t)slotIdx];
    case BattleTeam::Enemy: return enemyTarget[(size_t)slotIdx];
    default: break;
    }
    return _float3{};
}

_float2 BattleFormationSystem::GetFaceDir(BattleTeam team, int slotIdx) const
{
    switch (team)
    {
    case BattleTeam::Ally:  return allyFace[(size_t)slotIdx];
    case BattleTeam::Enemy: return enemyFace[(size_t)slotIdx];
    default: break;
    }
    return _float2{};
}

void BattleFormationSystem::ReComputeIntro(const FormationParams& p, float baseR, int leaderSlot, const vector<float>& allyAngles, const vector<float>& enemyAngles)
{
    const float enemyR = baseR;

    for (int i = 0; i < allyCount; ++i)
    {
        const bool  isLeader = (i == leaderSlot);
        const float r = isLeader ? baseR : baseR + max(0.f, p.backMeters);

        const float rad = XMConvertToRadians(allyAngles[(size_t)i]);
        allyTarget[(size_t)i] = _float3{
            centerWorld.x + sinf(rad) * r,
            centerWorld.y,
            centerWorld.z + cosf(rad) * r
        };
    }

    for (int i = 0; i < enemyCount; ++i)
    {
        const float rad = XMConvertToRadians(enemyAngles[(size_t)i]);
        enemyTarget[(size_t)i] = _float3{
            centerWorld.x + sinf(rad) * enemyR,
            centerWorld.y,
            centerWorld.z + cosf(rad) * enemyR
        };
    }

    ComputeFacing();
}

void BattleFormationSystem::ReComputeBattle(const FormationParams& p, float baseR, int leaderSlot,
    const vector<float>& allyAngles, const vector<float>& enemyAngles)
{
    const float enemyR = baseR;

    const float charDiameter = p.charRadiusMeters * 2.f;
    const float minTeamGap = charDiameter + 40.f;             // 팀끼리 기본 간격 조금 키움
    const float userGap = (p.backMeters != 0.f) ? fabsf(p.backMeters) : 120.f;
    const float teamGap = max(minTeamGap, userGap);

    const float allyFrontR = enemyR + teamGap * 0.7f;         // 리더: 적보다 꽤 앞
    const float allyBackR = enemyR + teamGap * 1.1f;         // 나머지: 더 뒤에서 받쳐줌

    for (int i = 0; i < enemyCount; ++i)
    {
        const float rad = XMConvertToRadians(enemyAngles[(size_t)i]);
        enemyTarget[(size_t)i] = _float3{
            centerWorld.x + sinf(rad) * enemyR,
            centerWorld.y,
            centerWorld.z + cosf(rad) * enemyR
        };
    }

    for (int i = 0; i < allyCount; ++i)
    {
        const bool  isLeader = (i == leaderSlot);
        const float r = isLeader ? allyFrontR : allyBackR;

        const float rad = XMConvertToRadians(allyAngles[(size_t)i]);
        allyTarget[(size_t)i] = _float3{
            centerWorld.x + sinf(rad) * r,
            centerWorld.y,
            centerWorld.z + cosf(rad) * r
        };
    }

    ComputeFacing();
}

void BattleFormationSystem::ComputeFacing()
{
    const int pairCount = min(allyCount, enemyCount);

    for (int i = 0; i < pairCount; ++i)
    {
        const _float3 a = allyTarget[(size_t)i];
        const _float3 b = enemyTarget[(size_t)i];

        const float dx = b.x - a.x;
        const float dz = b.z - a.z;

        const _float2 dirA = Utility::Normalize(dx, dz);
        allyFace[(size_t)i] = dirA;
        enemyFace[(size_t)i] = _float2{ -dirA.x, -dirA.y };
    }

    if (allyCount > pairCount)
    {
        _float3 target = centerWorld;
        if (enemyCount > 0)
        {
            _float3 acc{};
            for (int i = 0; i < enemyCount; ++i)
            {
                acc.x += enemyTarget[(size_t)i].x;
                acc.z += enemyTarget[(size_t)i].z;
            }
            acc.x /= enemyCount;
            acc.z /= enemyCount;
            target = acc;
        }

        for (int i = pairCount; i < allyCount; ++i)
        {
            const _float3 a = allyTarget[(size_t)i];
            allyFace[(size_t)i] = Utility::Normalize(target.x - a.x, target.z - a.z);
        }
    }

    if (enemyCount > pairCount)
    {
        _float3 target = centerWorld;
        if (allyCount > 0)
        {
            _float3 acc{};
            for (int i = 0; i < allyCount; ++i)
            {
                acc.x += allyTarget[(size_t)i].x;
                acc.z += allyTarget[(size_t)i].z;
            }
            acc.x /= allyCount;
            acc.z /= allyCount;
            target = acc;
        }

        for (int i = pairCount; i < enemyCount; ++i)
        {
            const _float3 b = enemyTarget[(size_t)i];
            enemyFace[(size_t)i] = Utility::Normalize(target.x - b.x, target.z - b.z);
        }
    }
}

void BattleFormationSystem::Init(const _float3 center, int allyCount, int enemyCount, const FormationParams& introParam, const FormationParams& battleParam)
{
    centerWorld = center;
    this->allyCount = allyCount;
    this->enemyCount = enemyCount;

    introParams = introParam;
    battleParams = battleParam;

    phase = FormationPhase::Intro;

    dirty = true;
    ReCompute();
}

void BattleFormationSystem::SetPhase(FormationPhase newPhase)
{
    if (phase == newPhase) return;
    phase = newPhase;
    dirty = true;
}

void BattleFormationSystem::SetParams(FormationPhase which, const FormationParams& params)
{
    if (which == FormationPhase::Intro)
        introParams = params;
    else
        battleParams = params;

    dirty = true;
}

void BattleFormationSystem::SetCenter(const _float3& center)
{
    centerWorld = center;
    dirty = true;
}

void BattleFormationSystem::ReCompute()
{
    dirty = false;

    for (int i = 0; i < 3; ++i)
    {
        allyTarget[i] = _float3{};
        enemyTarget[i] = _float3{};
        allyFace[i] = _float2{ 0.f, -1.f };
        enemyFace[i] = _float2{ 0.f, 1.f };
    }

    const FormationParams& p = (phase == FormationPhase::Intro) ? introParams : battleParams;

    const float needAllyR = TeamRequiredRadius(allyCount, p.allySpanDeg, p.charRadiusMeters, p.padDeg);
    const float needEnemyR = TeamRequiredRadius(enemyCount, p.enemySpanDeg, p.charRadiusMeters, p.padDeg);
    const float baseR = max(p.ringRadius, max(needAllyR, needEnemyR));

    int leaderSlot = p.allyLeaderSlot;
    if (leaderSlot < 0)
        leaderSlot = (allyCount == 3 ? 1 : 0);
    leaderSlot = clamp(leaderSlot, 0, max(0, allyCount - 1));

    vector<float> allyAngles = MakeAngles(allyCount, p.allyStartDeg, p.allySpanDeg);
    vector<float> enemyAngles = MakeAngles(enemyCount, p.enemyStartDeg, p.enemySpanDeg);

    if (phase == FormationPhase::Intro)
        ReComputeIntro(p, baseR, leaderSlot, allyAngles, enemyAngles);
    else
        ReComputeBattle(p, baseR, leaderSlot, allyAngles, enemyAngles);

    effectiveRadius = 0.f;

    for (int i = 0; i < allyCount; ++i)
    {
        _float3 v = allyTarget[(size_t)i];
        v.x -= centerWorld.x;
        v.z -= centerWorld.z;
        effectiveRadius = max(effectiveRadius, LengthXZ(v));
    }
    for (int i = 0; i < enemyCount; ++i)
    {
        _float3 v = enemyTarget[(size_t)i];
        v.x -= centerWorld.x;
        v.z -= centerWorld.z;
        effectiveRadius = max(effectiveRadius, LengthXZ(v));
    }
}