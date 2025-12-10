#include "Enginepch.h"
#include "CamSerializer.h"

namespace
{
    constexpr _uint CAM_MAGIC =
        static_cast<_uint>('C')
        | (static_cast<_uint>('A') << 8)
        | (static_cast<_uint>('M') << 16)
        | (static_cast<_uint>('0') << 24);

    constexpr _uint CAM_VERSION = static_cast<_uint>(CamFileVersion::V2);
}

void CamSerializer::OnBoot()
{
    camReg = &registry.Get<CamRegistry>();
}

bool CamSerializer::Save(ClipId clipId, const SeqCamPreset& preset, float baseFovDeg, const FollowTrackDesc& follow, const filesystem::path& path)
{
    ofstream out = BinaryUtil::OpenOut(path, true);
    if (!out) return false;

    BinaryUtil::WriteHeader(out, CAM_MAGIC, CAM_VERSION);

    _uint clip = static_cast<_uint>(clipId);
    BinaryUtil::WritePOD(out, clip);
    BinaryUtil::WritePOD(out, preset.duration);

    _uint keyCount = static_cast<_uint>(preset.keys.size());
    _uint shakeCount = static_cast<_uint>(preset.shakes.size());
    _uint clipCount = static_cast<_uint>(preset.clips.size());

    BinaryUtil::WritePOD(out, keyCount);
    BinaryUtil::WritePOD(out, shakeCount);
    BinaryUtil::WritePOD(out, clipCount);

    for (const CamKey& k : preset.keys)
    {
        BinaryUtil::WritePOD(out, k.t);
        BinaryUtil::WritePOD(out, k.pos);
        BinaryUtil::WritePOD(out, k.look);
        BinaryUtil::WritePOD(out, k.fovDeg);
    }

    for (const CamShakeEvent& s : preset.shakes)
    {
        BinaryUtil::WritePOD(out, s.t);
        BinaryUtil::WritePOD(out, s.width);
        BinaryUtil::WritePOD(out, s.amp);
        BinaryUtil::WritePOD(out, s.yScale);
    }

    for (const ShotClip& c : preset.clips)
    {
        BinaryUtil::WritePOD(out, c.t0);
        BinaryUtil::WritePOD(out, c.t1);
    }

    BinaryUtil::WritePOD(out, preset.meta);
    BinaryUtil::WritePOD(out, baseFovDeg);
    BinaryUtil::WritePOD(out, follow);

    BinaryUtil::WriteAlign(out, 16);
    return static_cast<bool>(out);
}

bool CamSerializer::Load(ClipId& outClipId, SeqCamPreset& outPreset, float& outBaseFovDeg, FollowTrackDesc& outFollow, const filesystem::path& path)
{
    ifstream in = BinaryUtil::OpenIn(path);
    if (!in) return false;

    _uint version = 0;
    if (!BinaryUtil::ReadHeader(in, CAM_MAGIC, version))
        return false;

    if (version != static_cast<_uint>(CamFileVersion::V1) &&
        version != static_cast<_uint>(CamFileVersion::V2))
        return false;

    SeqCamPreset preset{};

    _uint clip = 0;
    if (!BinaryUtil::ReadPOD(in, clip))            return false;
    if (!BinaryUtil::ReadPOD(in, preset.duration)) return false;

    _uint keyCount = 0;
    _uint shakeCount = 0;
    _uint clipCount = 0;
    if (!BinaryUtil::ReadPOD(in, keyCount))   return false;
    if (!BinaryUtil::ReadPOD(in, shakeCount)) return false;
    if (!BinaryUtil::ReadPOD(in, clipCount))  return false;

    // --- CamKey 읽기: 버전에 따라 레이아웃 다르게 -----
    preset.keys.resize(keyCount);
    for (_uint i = 0; i < keyCount; ++i)
    {
        CamKey k{};

        if (!BinaryUtil::ReadPOD(in, k.t))   return false;
        if (!BinaryUtil::ReadPOD(in, k.pos)) return false;

        if (version >= static_cast<_uint>(CamFileVersion::V2))
        {
            // V2: look까지 있음
            if (!BinaryUtil::ReadPOD(in, k.look)) return false;
        }
        else
        {
            // V1: look 정보가 없으니까 기본값 (0,0,0)
            k.look = _float3{};
        }

        if (!BinaryUtil::ReadPOD(in, k.fovDeg)) return false;

        preset.keys[i] = k;
    }

    // 나머지는 버전 공통
    preset.shakes.resize(shakeCount);
    for (_uint i = 0; i < shakeCount; ++i)
    {
        CamShakeEvent s{};
        if (!BinaryUtil::ReadPOD(in, s.t))      return false;
        if (!BinaryUtil::ReadPOD(in, s.width))  return false;
        if (!BinaryUtil::ReadPOD(in, s.amp))    return false;
        if (!BinaryUtil::ReadPOD(in, s.yScale)) return false;
        preset.shakes[i] = s;
    }

    preset.clips.resize(clipCount);
    for (_uint i = 0; i < clipCount; ++i)
    {
        ShotClip c{};
        if (!BinaryUtil::ReadPOD(in, c.t0)) return false;
        if (!BinaryUtil::ReadPOD(in, c.t1)) return false;
        preset.clips[i] = c;
    }

    if (!BinaryUtil::ReadPOD(in, preset.meta))     return false;
    if (!BinaryUtil::ReadPOD(in, outBaseFovDeg))   return false;
    if (!BinaryUtil::ReadPOD(in, outFollow))       return false;

    if (preset.clips.empty() && preset.duration > 0.f)
    {
        ShotClip full{};
        full.t0 = 0.0;
        full.t1 = preset.duration;
        preset.clips.push_back(full);
    }

    float dur = camReg->ComputeCamDuration(preset);
    preset.duration = dur;

    if (preset.clips.empty() && dur > 0.f)
    {
        ShotClip full{};
        full.t0 = 0.0f;
        full.t1 = dur;
        preset.clips.push_back(full);
    }

    BinaryUtil::ReadAlign(in, 16);

    outClipId = static_cast<ClipId>(clip);
    outPreset = std::move(preset);
    return true;
}