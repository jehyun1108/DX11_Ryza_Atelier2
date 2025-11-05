#pragma once

NS_BEGIN(Engine)

struct Lens
{
    float fovY = 65.f;
    float nearZ = 0.1f;
    float farZ = 3000.f;
};

struct AnchorBinding
{
    AnchorSpace   space = AnchorSpace::World;
    TargetBinding binding = TargetBinding::None;
    EntityID      entity = 0;
    _float3       offset = {};
};

struct CamPose
{
    _float3 pos{};
    _float4 rot{};
    Lens    lens{};
};

struct FollowTrackDesc
{
    float orbitRadius    = 300.f;
    float orbitHeight    = 200.f;
    float yawOffsetDeg   = 0.f;
    float dampingDegPerS = 0.f;     // 0이면 미사용(궤도 각 직접 산출)
    float speedYawClamp  = 360.f;   // deg/s

    float aimOffsetY = 50.f;
};

using ClipId = _uint;

struct SequenceTrackDesc
{
    ClipId clipId       = 0;
    bool   lockUntilEnd = false;
    bool   loop         = false;
    float  timeScale    = 1.f;
};

struct ShotClip
{
    double t0 = 0.0;
    double t1 = 0.0;
};

struct SequenceRuntime
{
    vector<ShotClip> clips;
    size_t  curIdx       = 0;
    double  localTime    = 0.0;
    bool    loop         = false;
    float   timeScale    = 1.f;
    bool    lockUntilEnd = false;
};

struct TrackID
{
    _uint idx = 0;
    _uint gen = 0;
    bool IsValid() const { return gen != 0; }
};

struct SmoothingConfig
{
    float halfLifePosSec  = 0.30f;  
    float halfLifeRotSec  = 0.30f;   
    float epsilonPos      = 0.02f;   
    float epsilonRotDeg   = 0.5f;    
    float maxPosMps       = 600.f;  
    float maxRotDegPerSec = 300.f;  
};

struct TrackState
{
    CamTrackType  type     = CamTrackType::Follow;
    CamPriority   priority = CamPriority::Default;
    CamLayer      layer    = CamLayer::Base;

    AnchorBinding anchor{};
    TrackPhase    phase = TrackPhase::Inactive;
    float         activeW = 0.f;    

    CamPose goal{};
    CamPose cur{};

    FollowTrackDesc   followDesc{};
    SequenceTrackDesc seqDesc{};
    SequenceRuntime   seqRt{};

    bool     valid = false;
    TrackID  id{};
};

struct MixerGroup
{
    CamPriority priority = CamPriority::Default;
    std::vector<_uint> trackIndices;
};

struct DirectorState
{
    double  elapsed = 0.0;
    CamPose output{};
    bool    hasOutput = false;

    Lens                fixedLens{};
    std::vector<TrackState> tracks;
    std::vector<MixerGroup> groups;
};

struct TrackSpawnRequest
{
    CamTrackType   type = CamTrackType::Sequence;
    CamPriority    priority = CamPriority::High;
    CamLayer       layer = CamLayer::Action;

    AnchorBinding  anchor{};
    FollowTrackDesc   followDesc{};
    SequenceTrackDesc seqDesc{};
};

struct TrackKillRequest
{
    TrackID id{};
    bool    immediate = false; // true면 즉시 제거, false면 FadingOut
};
// ===================================================================
NS_END