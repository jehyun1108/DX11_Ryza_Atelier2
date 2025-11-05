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
// ---------------------------------------------------------------------------------------------------------------------------------
void BattleFormationSystem::Init(const _float3 center, int allyCount, int enemyCount, const FormationParams& fParams)
{
	centerWorld       = center;
	this->allyCount   = clamp(allyCount, 0, 3);
	this->enemyCount  = clamp(enemyCount, 0, 3);
	params            = fParams;
	dirty             = true;
	ReCompute();
}

bool BattleFormationSystem::TryGetTargetPos(BattleTeam team, int slotIdx, _float3& out) const
{
    if (slotIdx < 0 || slotIdx >= 3) return false;
    if (team == BattleTeam::Ally)
    {
        if (slotIdx >= allyCount) return false;
        out = allyTarget[(size_t)slotIdx];
        return true;
    }
    if (team == BattleTeam::Enemy)
    {
        if (slotIdx >= enemyCount) return false;
        out = enemyTarget[(size_t)slotIdx];
        return true;
    }
    return false;
}

bool BattleFormationSystem::TryGetFaceDir(BattleTeam team, int slotIdx, _float2& out) const
{
    if (slotIdx < 0 || slotIdx >= 3) return false;
    if (team == BattleTeam::Ally)
    {
        if (slotIdx >= allyCount) return false;
        out = allyFace[(size_t)slotIdx];
        return true;
    }
    if (team == BattleTeam::Enemy)
    {
        if (slotIdx >= enemyCount) return false;
        out = enemyFace[(size_t)slotIdx];
        return true;
    }
    return false;
}

void BattleFormationSystem::ReCompute()
{
    dirty = false;

    for (int i = 0; i < 3; ++i)
    {
        allyTarget[i]  = _float3{};
        enemyTarget[i] = _float3{};
        allyFace[i]    = _float2{ 0.f, -1.f };
        enemyFace[i]   = _float2{ 0.f, 1.f };
    }

    float needAllyR  = TeamRequiredRadius(allyCount,  params.allySpanDeg,  params.charRadiusMeters, params.padDeg);
    float needEnemyR = TeamRequiredRadius(enemyCount, params.enemySpanDeg, params.charRadiusMeters, params.padDeg);

    effectiveRadius = max(params.ringRadius, max(needAllyR, needEnemyR));
    effectiveRadius = max(1.f, effectiveRadius);

    vector<float> allyAngles  = MakeAngles(allyCount, params.allyStartDeg, params.allySpanDeg);
    vector<float> enemyAngles = MakeAngles(enemyCount, params.enemyStartDeg, params.enemySpanDeg);

    for (int i = 0; i < allyCount; ++i)  allyTarget[(size_t)i]  = PosOnRing(centerWorld, allyAngles[(size_t)i],  effectiveRadius);
    for (int i = 0; i < enemyCount; ++i) enemyTarget[(size_t)i] = PosOnRing(centerWorld, enemyAngles[(size_t)i], effectiveRadius);

    const int pair = min(allyCount, enemyCount);
    for (int i = 0; i < pair; ++i)
    {
        const _float3 a      = allyTarget[(size_t)i];
        const _float3 b      = enemyTarget[(size_t)i];
        const _float2 dirA   = Utility::Normalize(b.x - a.x, b.z - a.z);
        const _float2 dirB   = _float2{ -dirA.x, -dirA.y };
        allyFace[(size_t)i]  = dirA;
        enemyFace[(size_t)i] = dirB;
    }

    if (allyCount > pair)
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
        for (int i = pair; i < allyCount; ++i)
        {
            const _float3 a     = allyTarget[(size_t)i];
            const _float2 dir   = Utility::Normalize(target.x - a.x, target.z - a.z);
            allyFace[(size_t)i] = dir;
        }
    }

    if (enemyCount > pair)
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
        for (int i = pair; i < enemyCount; ++i)
        {
            const _float3 b      = enemyTarget[(size_t)i];
            const _float2 dir    = Utility::Normalize(target.x - b.x, target.z - b.z);
            enemyFace[(size_t)i] = dir;
        }
    }
}