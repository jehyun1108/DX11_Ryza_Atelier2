#include "Enginepch.h"
#include "EffectSerializer.h"

namespace
{
    constexpr _uint EFFECT_MAGIC =
         static_cast<_uint>('E') |
        (static_cast<_uint>('F') << 8) |
        (static_cast<_uint>('T') << 16) |
        (static_cast<_uint>('0') << 24);
}
 // ==========================================================
bool EffectSerializer::WriteWString(ofstream& outFile, const wstring& wstr)
{
    string str = Utility::ToString(wstr);
    BinaryUtil::WriteString(outFile, str);
    return static_cast<bool>(outFile);
}

bool EffectSerializer::ReadWString(ifstream& inFile, wstring& out)
{
    string str;
    if (!BinaryUtil::ReadString(inFile, str))
        return false;

    out = Utility::ToWString(str);
    return true;
}

bool EffectSerializer::Save(const EffectArchetype& effect, const filesystem::path& path)
{
    ofstream out = BinaryUtil::OpenOut(path, true);
    if (!out) return false;

    BinaryUtil::WriteHeader(out, EFFECT_MAGIC, EFFECT_VERSION);

    if (!WriteWString(out, effect.key)) return false;
    BinaryUtil::WritePOD(out, effect.duration);

    uint8_t layer = static_cast<uint8_t>(effect.layer);
    BinaryUtil::WritePOD(out, layer);

    _uint emitterCount = static_cast<_uint>(effect.emitters.size());
    _uint eventCount = static_cast<_uint>(effect.events.size());
    BinaryUtil::WritePOD(out, emitterCount);
    BinaryUtil::WritePOD(out, eventCount);

    for (const auto& em : effect.emitters)
    {
        if (!WriteWString(out, em.name)) return false;
        BinaryUtil::WritePOD(out, em.localOffset);

        uint8_t spaceMode = static_cast<uint8_t>(em.spaceMode);
        BinaryUtil::WritePOD(out, spaceMode);

        uint8_t kind = static_cast<uint8_t>(em.kind);
        BinaryUtil::WritePOD(out, kind);

        uint8_t burstFlag = em.burst ? 1u : 0u;
        BinaryUtil::WritePOD(out, burstFlag);

        int32_t burstCount = static_cast<int32_t>(em.burstCount);
        BinaryUtil::WritePOD(out, burstCount);

        BinaryUtil::WritePOD(out, em.delay);
        BinaryUtil::WritePOD(out, em.duration);

        uint8_t useCollider = em.useColliderObbTip ? 1u : 0u;
        BinaryUtil::WritePOD(out, useCollider);

        // ── ParticleSpawnData (V3 전체) ─────────────────
        WriteParticleV3(out, em.particle);

        // ── TrailDesc (V3 전체) ────────────────────────
        const TrailDesc& td = em.trail;

        BinaryUtil::WritePOD(out, td.lifeTime);
        BinaryUtil::WritePOD(out, td.widthStart);
        BinaryUtil::WritePOD(out, td.widthEnd);

        BinaryUtil::WritePOD(out, td.colorStart);
        BinaryUtil::WritePOD(out, td.colorEnd);

        uint8_t widthCurveT = static_cast<uint8_t>(td.widthCurve);
        uint8_t alphaCurveT = static_cast<uint8_t>(td.alphaCurve);
        BinaryUtil::WritePOD(out, widthCurveT);
        BinaryUtil::WritePOD(out, alphaCurveT);
        BinaryUtil::WritePOD(out, td.minSegDist);

        if (!WriteWString(out, td.texKey)) return false;

        uint8_t shapeMode = static_cast<uint8_t>(td.shapeMode);
        BinaryUtil::WritePOD(out, shapeMode);

        uint8_t arcPlane = static_cast<uint8_t>(td.arcPlane);
        BinaryUtil::WritePOD(out, arcPlane);

        BinaryUtil::WritePOD(out, td.arcRadius);
        BinaryUtil::WritePOD(out, td.arcStartDeg);
        BinaryUtil::WritePOD(out, td.arcEndDeg);

        uint8_t useOwnerCenter = td.arcUseownerCenter ? 1u : 0u;
        BinaryUtil::WritePOD(out, useOwnerCenter);

        BinaryUtil::WritePOD(out, td.arcCenterOffset);
        BinaryUtil::WritePOD(out, td.arcRotDeg);

        // ── Trail Spark 관련 필드 (V3 추가) ─────────────
        uint8_t sparkEnabled = td.sparkEnabled ? 1u : 0u;
        BinaryUtil::WritePOD(out, sparkEnabled);
        BinaryUtil::WritePOD(out, td.sparkInterval);

        int32_t sparkBurst = static_cast<int32_t>(td.sparkBurstCount);
        BinaryUtil::WritePOD(out, sparkBurst);

        // spark 자체도 ParticleSpawnData 구조
        WriteParticleV3(out, td.spark);
    }

    // Events --------------------------------------------------
    for (const auto& ev : effect.events)
    {
        BinaryUtil::WritePOD(out, ev.time);

        uint8_t type = static_cast<uint8_t>(ev.type);
        BinaryUtil::WritePOD(out, type);

        int32_t emitterIdx = static_cast<int32_t>(ev.emitterIdx);
        BinaryUtil::WritePOD(out, emitterIdx);
    }

    return static_cast<bool>(out);
}

bool EffectSerializer::Load(EffectArchetype& out, const filesystem::path& path)
{
    ifstream in = BinaryUtil::OpenIn(path);
    if (!in) return false;

    _uint version = 0;
    if (!BinaryUtil::ReadHeader(in, EFFECT_MAGIC, version))
        return false;

    if (version == static_cast<_uint>(EffectFileVersion::V1))
        return LoadV2(out, in);

    if (version == static_cast<_uint>(EffectFileVersion::V2))
        return LoadV2(out, in);

    if (version == static_cast<_uint>(EffectFileVersion::V3))
        return LoadV3(out, in);

    return false;
}

bool EffectSerializer::LoadV2(EffectArchetype& out, ifstream& in)
{
    EffectArchetype tmp{};

    if (!ReadWString(in, tmp.key)) return false;

    if (!BinaryUtil::ReadPOD(in, tmp.duration)) return false;

    uint8_t layer = 0;
    if (!BinaryUtil::ReadPOD(in, layer)) return false;
    tmp.layer = static_cast<EffectRenderLayer>(layer);

    _uint emitterCount = 0;
    _uint eventCount = 0;
    if (!BinaryUtil::ReadPOD(in, emitterCount)) return false;
    if (!BinaryUtil::ReadPOD(in, eventCount))   return false;

    tmp.emitters.resize(emitterCount);
    tmp.events.resize(eventCount);

    for (_uint i = 0; i < emitterCount; ++i)
    {
        EffectEmitterDesc& em = tmp.emitters[i];

        if (!ReadWString(in, em.name)) return false;
        if (!BinaryUtil::ReadPOD(in, em.localOffset)) return false;

        uint8_t spaceMode = 0;
        if (!BinaryUtil::ReadPOD(in, spaceMode)) return false;
        em.spaceMode = static_cast<EffectSpaceMode>(spaceMode);

        uint8_t kind = 0;
        if (!BinaryUtil::ReadPOD(in, kind)) return false;
        em.kind = static_cast<EffectEmitterKind>(kind);

        uint8_t burstFlag = 0;
        if (!BinaryUtil::ReadPOD(in, burstFlag)) return false;
        em.burst = (burstFlag != 0);

        int32_t burstCount = 0;
        if (!BinaryUtil::ReadPOD(in, burstCount)) return false;
        em.burstCount = burstCount;

        if (!BinaryUtil::ReadPOD(in, em.delay))    return false;
        if (!BinaryUtil::ReadPOD(in, em.duration)) return false;

        uint8_t useCollider = 0;
        if (!BinaryUtil::ReadPOD(in, useCollider)) return false;
        em.useColliderObbTip = (useCollider != 0);

        ParticleSpawnData& sd = em.particle;

        if (!BinaryUtil::ReadPOD(in, sd.spawnRate)) return false;

        if (!BinaryUtil::ReadPOD(in, sd.lifeMin))   return false;
        if (!BinaryUtil::ReadPOD(in, sd.lifeMax))   return false;
        if (!BinaryUtil::ReadPOD(in, sd.speedMin))  return false;
        if (!BinaryUtil::ReadPOD(in, sd.speedMax))  return false;

        if (!BinaryUtil::ReadPOD(in, sd.baseDir))   return false;
        if (!BinaryUtil::ReadPOD(in, sd.spreadAng)) return false;

        if (!BinaryUtil::ReadPOD(in, sd.startSize)) return false;
        if (!BinaryUtil::ReadPOD(in, sd.endSize))   return false;

        if (!BinaryUtil::ReadPOD(in, sd.startColor)) return false;
        if (!BinaryUtil::ReadPOD(in, sd.endColor))   return false;

        uint8_t sizeCurve = 0;
        uint8_t alphaCurve = 0;
        uint8_t colorCurve = 0;
        uint8_t rateCurve = 0;

        if (!BinaryUtil::ReadPOD(in, sizeCurve))  return false;
        if (!BinaryUtil::ReadPOD(in, alphaCurve)) return false;
        if (!BinaryUtil::ReadPOD(in, colorCurve)) return false;
        if (!BinaryUtil::ReadPOD(in, rateCurve))  return false;

        sd.sizeCurve = static_cast<EffectCurveType>(sizeCurve);
        sd.alphaCurve = static_cast<EffectCurveType>(alphaCurve);
        sd.colorCurve = static_cast<EffectCurveType>(colorCurve);
        sd.rateCurve = static_cast<EffectCurveType>(rateCurve);

        if (!ReadWString(in, sd.texKey)) return false;

        TrailDesc& td = em.trail;

        if (!BinaryUtil::ReadPOD(in, td.lifeTime))    return false;
        if (!BinaryUtil::ReadPOD(in, td.widthStart))  return false;
        if (!BinaryUtil::ReadPOD(in, td.widthEnd))    return false;

        if (!BinaryUtil::ReadPOD(in, td.colorStart))  return false;
        if (!BinaryUtil::ReadPOD(in, td.colorEnd))    return false;

        uint8_t widthCurveT = 0;
        uint8_t alphaCurveT = 0;
        if (!BinaryUtil::ReadPOD(in, widthCurveT)) return false;
        if (!BinaryUtil::ReadPOD(in, alphaCurveT)) return false;
        td.widthCurve = static_cast<EffectCurveType>(widthCurveT);
        td.alphaCurve = static_cast<EffectCurveType>(alphaCurveT);

        if (!BinaryUtil::ReadPOD(in, td.minSegDist))  return false;

        if (!ReadWString(in, td.texKey)) return false;

        uint8_t shapeMode = 0;
        if (!BinaryUtil::ReadPOD(in, shapeMode)) return false;
        td.shapeMode = static_cast<TrailShapeMode>(shapeMode);

        uint8_t arcPlane = 0;
        if (!BinaryUtil::ReadPOD(in, arcPlane)) return false;
        td.arcPlane = static_cast<TrailArcPlane>(arcPlane);

        if (!BinaryUtil::ReadPOD(in, td.arcRadius))       return false;
        if (!BinaryUtil::ReadPOD(in, td.arcStartDeg))     return false;
        if (!BinaryUtil::ReadPOD(in, td.arcEndDeg))       return false;

        uint8_t useOwnerCenter = 0;
        if (!BinaryUtil::ReadPOD(in, useOwnerCenter)) return false;
        td.arcUseownerCenter = (useOwnerCenter != 0);

        if (!BinaryUtil::ReadPOD(in, td.arcCenterOffset)) return false;
        if (!BinaryUtil::ReadPOD(in, td.arcRotDeg))       return false;
    }

    for (_uint i = 0; i < eventCount; ++i)
    {
        EffectEventDesc& ev = tmp.events[i];

        if (!BinaryUtil::ReadPOD(in, ev.time)) return false;

        uint8_t type = 0;
        if (!BinaryUtil::ReadPOD(in, type)) return false;
        ev.type = static_cast<EffectEventType>(type);

        int32_t emitterIdx = -1;
        if (!BinaryUtil::ReadPOD(in, emitterIdx)) return false;
        ev.emitterIdx = emitterIdx;
    }

    // ── V2 → V3 호환용 기본값 채워주기 ─────────────────
    for (auto& em : tmp.emitters)
    {
        ParticleSpawnData& sd = em.particle;

        sd.visualMode = ParticleVisualMode::Billboard;
        sd.sheet = {};      // enabled=false, cols/rows=1 ...
        sd.dirLocal = false;
        sd.rotSpeedMin = 0.f;
        sd.rotSpeedMax = 0.f;
        sd.randomStartRot = false;
        sd.posRadiusMin = 0.f;
        sd.posRadiusMax = 0.f;
        sd.presetType = EffectPresetType::None;

        TrailDesc& td = em.trail;
        td.sparkEnabled = false;
        td.sparkInterval = 0.02f;
        td.sparkBurstCount = 4;
        td.spark = {}; // 기본 ParticleSpawnData
    }

    out = std::move(tmp);
    return true;
}

bool EffectSerializer::LoadV3(EffectArchetype& out, ifstream& in)
{
    EffectArchetype tmp{};

    if (!ReadWString(in, tmp.key)) return false;

    if (!BinaryUtil::ReadPOD(in, tmp.duration)) return false;

    uint8_t layer = 0;
    if (!BinaryUtil::ReadPOD(in, layer)) return false;
    tmp.layer = static_cast<EffectRenderLayer>(layer);

    _uint emitterCount = 0;
    _uint eventCount = 0;
    if (!BinaryUtil::ReadPOD(in, emitterCount)) return false;
    if (!BinaryUtil::ReadPOD(in, eventCount))   return false;

    tmp.emitters.resize(emitterCount);
    tmp.events.resize(eventCount);

    for (_uint i = 0; i < emitterCount; ++i)
    {
        EffectEmitterDesc& em = tmp.emitters[i];

        if (!ReadWString(in, em.name)) return false;
        if (!BinaryUtil::ReadPOD(in, em.localOffset)) return false;

        uint8_t spaceMode = 0;
        if (!BinaryUtil::ReadPOD(in, spaceMode)) return false;
        em.spaceMode = static_cast<EffectSpaceMode>(spaceMode);

        uint8_t kind = 0;
        if (!BinaryUtil::ReadPOD(in, kind)) return false;
        em.kind = static_cast<EffectEmitterKind>(kind);

        uint8_t burstFlag = 0;
        if (!BinaryUtil::ReadPOD(in, burstFlag)) return false;
        em.burst = (burstFlag != 0);

        int32_t burstCount = 0;
        if (!BinaryUtil::ReadPOD(in, burstCount)) return false;
        em.burstCount = burstCount;

        if (!BinaryUtil::ReadPOD(in, em.delay))    return false;
        if (!BinaryUtil::ReadPOD(in, em.duration)) return false;

        uint8_t useCollider = 0;
        if (!BinaryUtil::ReadPOD(in, useCollider)) return false;
        em.useColliderObbTip = (useCollider != 0);

        // Particle V3
        if (!ReadParticleV3(in, em.particle)) return false;

        // TrailDesc V3
        TrailDesc& td = em.trail;

        if (!BinaryUtil::ReadPOD(in, td.lifeTime))    return false;
        if (!BinaryUtil::ReadPOD(in, td.widthStart))  return false;
        if (!BinaryUtil::ReadPOD(in, td.widthEnd))    return false;

        if (!BinaryUtil::ReadPOD(in, td.colorStart))  return false;
        if (!BinaryUtil::ReadPOD(in, td.colorEnd))    return false;

        uint8_t widthCurveT = 0;
        uint8_t alphaCurveT = 0;
        if (!BinaryUtil::ReadPOD(in, widthCurveT)) return false;
        if (!BinaryUtil::ReadPOD(in, alphaCurveT)) return false;
        td.widthCurve = static_cast<EffectCurveType>(widthCurveT);
        td.alphaCurve = static_cast<EffectCurveType>(alphaCurveT);

        if (!BinaryUtil::ReadPOD(in, td.minSegDist))  return false;

        if (!ReadWString(in, td.texKey)) return false;

        uint8_t shapeMode = 0;
        if (!BinaryUtil::ReadPOD(in, shapeMode)) return false;
        td.shapeMode = static_cast<TrailShapeMode>(shapeMode);

        uint8_t arcPlane = 0;
        if (!BinaryUtil::ReadPOD(in, arcPlane)) return false;
        td.arcPlane = static_cast<TrailArcPlane>(arcPlane);

        if (!BinaryUtil::ReadPOD(in, td.arcRadius))       return false;
        if (!BinaryUtil::ReadPOD(in, td.arcStartDeg))     return false;
        if (!BinaryUtil::ReadPOD(in, td.arcEndDeg))       return false;

        uint8_t useOwnerCenter = 0;
        if (!BinaryUtil::ReadPOD(in, useOwnerCenter)) return false;
        td.arcUseownerCenter = (useOwnerCenter != 0);

        if (!BinaryUtil::ReadPOD(in, td.arcCenterOffset)) return false;
        if (!BinaryUtil::ReadPOD(in, td.arcRotDeg))       return false;

        // spark
        uint8_t sparkEnabled = 0;
        if (!BinaryUtil::ReadPOD(in, sparkEnabled)) return false;
        td.sparkEnabled = (sparkEnabled != 0);

        if (!BinaryUtil::ReadPOD(in, td.sparkInterval))  return false;

        int32_t sparkBurst = 0;
        if (!BinaryUtil::ReadPOD(in, sparkBurst)) return false;
        td.sparkBurstCount = sparkBurst;

        if (!ReadParticleV3(in, td.spark)) return false;
    }

    for (_uint i = 0; i < eventCount; ++i)
    {
        EffectEventDesc& ev = tmp.events[i];

        if (!BinaryUtil::ReadPOD(in, ev.time)) return false;

        uint8_t type = 0;
        if (!BinaryUtil::ReadPOD(in, type)) return false;
        ev.type = static_cast<EffectEventType>(type);

        int32_t emitterIdx = -1;
        if (!BinaryUtil::ReadPOD(in, emitterIdx)) return false;
        ev.emitterIdx = emitterIdx;
    }

    out = std::move(tmp);
    return true;
}

void EffectSerializer::WriteParticleV3(ofstream& out, const ParticleSpawnData& sd)
{
    BinaryUtil::WritePOD(out, sd.spawnRate);

    BinaryUtil::WritePOD(out, sd.lifeMin);
    BinaryUtil::WritePOD(out, sd.lifeMax);
    BinaryUtil::WritePOD(out, sd.speedMin);
    BinaryUtil::WritePOD(out, sd.speedMax);

    BinaryUtil::WritePOD(out, sd.baseDir);
    BinaryUtil::WritePOD(out, sd.spreadAng);

    BinaryUtil::WritePOD(out, sd.startSize);
    BinaryUtil::WritePOD(out, sd.endSize);

    BinaryUtil::WritePOD(out, sd.startColor);
    BinaryUtil::WritePOD(out, sd.endColor);

    uint8_t sizeCurve = static_cast<uint8_t>(sd.sizeCurve);
    uint8_t alphaCurve = static_cast<uint8_t>(sd.alphaCurve);
    uint8_t colorCurve = static_cast<uint8_t>(sd.colorCurve);
    uint8_t rateCurve = static_cast<uint8_t>(sd.rateCurve);

    BinaryUtil::WritePOD(out, sizeCurve);
    BinaryUtil::WritePOD(out, alphaCurve);
    BinaryUtil::WritePOD(out, colorCurve);
    BinaryUtil::WritePOD(out, rateCurve);

    uint8_t preset = static_cast<uint8_t>(sd.presetType);
    BinaryUtil::WritePOD(out, preset);

    if (!WriteWString(out, sd.texKey)) return;

    // visual / 회전 / dirLocal / sheet / radius
    uint8_t visual = static_cast<uint8_t>(sd.visualMode);
    BinaryUtil::WritePOD(out, visual);

    uint8_t dirLocalFlag = sd.dirLocal ? 1u : 0u;
    BinaryUtil::WritePOD(out, dirLocalFlag);

    BinaryUtil::WritePOD(out, sd.rotSpeedMin);
    BinaryUtil::WritePOD(out, sd.rotSpeedMax);

    uint8_t randomRot = sd.randomStartRot ? 1u : 0u;
    BinaryUtil::WritePOD(out, randomRot);

    // SpriteSheetInfo
    uint8_t sheetEnabled = sd.sheet.enabled ? 1u : 0u;
    uint8_t sheetAnimate = sd.sheet.animate ? 1u : 0u;
    BinaryUtil::WritePOD(out, sheetEnabled);
    BinaryUtil::WritePOD(out, sheetAnimate);

    int32_t cols = sd.sheet.cols;
    int32_t rows = sd.sheet.rows;
    int32_t startF = sd.sheet.startFrame;
    int32_t endF = sd.sheet.endFrame;
    BinaryUtil::WritePOD(out, cols);
    BinaryUtil::WritePOD(out, rows);
    BinaryUtil::WritePOD(out, startF);
    BinaryUtil::WritePOD(out, endF);

    BinaryUtil::WritePOD(out, sd.sheet.fps);

    uint8_t sheetLoop = sd.sheet.loop ? 1u : 0u;
    BinaryUtil::WritePOD(out, sheetLoop);

    // position radius
    BinaryUtil::WritePOD(out, sd.posRadiusMin);
    BinaryUtil::WritePOD(out, sd.posRadiusMax);
}

bool EffectSerializer::ReadParticleV3(ifstream& in, ParticleSpawnData& sd)
{
    if (!BinaryUtil::ReadPOD(in, sd.spawnRate)) return false;

    if (!BinaryUtil::ReadPOD(in, sd.lifeMin))   return false;
    if (!BinaryUtil::ReadPOD(in, sd.lifeMax))   return false;
    if (!BinaryUtil::ReadPOD(in, sd.speedMin))  return false;
    if (!BinaryUtil::ReadPOD(in, sd.speedMax))  return false;

    if (!BinaryUtil::ReadPOD(in, sd.baseDir))   return false;
    if (!BinaryUtil::ReadPOD(in, sd.spreadAng)) return false;

    if (!BinaryUtil::ReadPOD(in, sd.startSize)) return false;
    if (!BinaryUtil::ReadPOD(in, sd.endSize))   return false;

    if (!BinaryUtil::ReadPOD(in, sd.startColor)) return false;
    if (!BinaryUtil::ReadPOD(in, sd.endColor))   return false;

    uint8_t sizeCurve = 0;
    uint8_t alphaCurve = 0;
    uint8_t colorCurve = 0;
    uint8_t rateCurve = 0;

    if (!BinaryUtil::ReadPOD(in, sizeCurve))  return false;
    if (!BinaryUtil::ReadPOD(in, alphaCurve)) return false;
    if (!BinaryUtil::ReadPOD(in, colorCurve)) return false;
    if (!BinaryUtil::ReadPOD(in, rateCurve))  return false;

    sd.sizeCurve = static_cast<EffectCurveType>(sizeCurve);
    sd.alphaCurve = static_cast<EffectCurveType>(alphaCurve);
    sd.colorCurve = static_cast<EffectCurveType>(colorCurve);
    sd.rateCurve = static_cast<EffectCurveType>(rateCurve);

    uint8_t preset = 0;
    if (!BinaryUtil::ReadPOD(in, preset)) return false;
    sd.presetType = static_cast<EffectPresetType>(preset);

    if (!ReadWString(in, sd.texKey)) return false;

    uint8_t visual = 0;
    if (!BinaryUtil::ReadPOD(in, visual)) return false;
    sd.visualMode = static_cast<ParticleVisualMode>(visual);

    uint8_t dirLocalFlag = 0;
    if (!BinaryUtil::ReadPOD(in, dirLocalFlag)) return false;
    sd.dirLocal = (dirLocalFlag != 0);

    if (!BinaryUtil::ReadPOD(in, sd.rotSpeedMin)) return false;
    if (!BinaryUtil::ReadPOD(in, sd.rotSpeedMax)) return false;

    uint8_t randomRot = 0;
    if (!BinaryUtil::ReadPOD(in, randomRot)) return false;
    sd.randomStartRot = (randomRot != 0);

    uint8_t sheetEnabled = 0;
    uint8_t sheetAnimate = 0;
    if (!BinaryUtil::ReadPOD(in, sheetEnabled)) return false;
    if (!BinaryUtil::ReadPOD(in, sheetAnimate)) return false;

    sd.sheet.enabled = (sheetEnabled != 0);
    sd.sheet.animate = (sheetAnimate != 0);

    int32_t cols = 0, rows = 0, startF = 0, endF = 0;
    if (!BinaryUtil::ReadPOD(in, cols))    return false;
    if (!BinaryUtil::ReadPOD(in, rows))    return false;
    if (!BinaryUtil::ReadPOD(in, startF))  return false;
    if (!BinaryUtil::ReadPOD(in, endF))    return false;

    sd.sheet.cols = cols;
    sd.sheet.rows = rows;
    sd.sheet.startFrame = startF;
    sd.sheet.endFrame = endF;

    if (!BinaryUtil::ReadPOD(in, sd.sheet.fps)) return false;

    uint8_t sheetLoop = 0;
    if (!BinaryUtil::ReadPOD(in, sheetLoop)) return false;
    sd.sheet.loop = (sheetLoop != 0);

    if (!BinaryUtil::ReadPOD(in, sd.posRadiusMin)) return false;
    if (!BinaryUtil::ReadPOD(in, sd.posRadiusMax)) return false;

    return true;
}
