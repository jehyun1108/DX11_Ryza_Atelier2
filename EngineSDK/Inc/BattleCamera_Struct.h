#pragma once

NS_BEGIN(Engine)

using ClipId = _uint;

struct Lens
{
    float fovY  = 60.f;
    float nearZ = 0.1f;
    float farZ  = 5000.f;
};
struct AnchorBinding
{
    AnchorSpace   space   = AnchorSpace::World;
    TargetBinding binding = TargetBinding::None;
    EntityID      entity  = 0;
    _float3       offset  = {};
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
    float dampingDegPerS = 0.f;   
    float speedYawClamp  = 360.f;   
    float aimOffsetY     = 50.f;
};
struct SequenceTrackDesc
{
    ClipId clipId       = 0;
    bool   lockUntilEnd = false;
    bool   loop         = false;
    float  timeScale    = 1.f;
};
struct ShotClip
{
    float t0 = 0.f;
    float t1 = 0.f;
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
};
struct TrackState
{
    CamTrackType  type     = CamTrackType::Follow;
    CamPriority   priority = CamPriority::Default;
    CamLayer      layer    = CamLayer::Base;

    AnchorBinding anchor{};  

    CamPose       goal{};
    FollowTrackDesc   followDesc{};
    SequenceTrackDesc seqDesc{};
    SequenceRuntime   seqRt{};

    bool     valid = false;
    TrackID  id{};

    bool  dying = false;
    float fadeOutSec = 0.f;
    float fadeOutRemain = 0.f;

    bool    hasFollowHistory = false;
    _float3 followCenter{}; 
    _float3 followFocus{}; 
};
struct MixerGroup
{
    CamPriority   priority = CamPriority::Default;
    vector<_uint> trackIndices;
};
struct DirectorState
{
    double             elapsed = 0.0;
    CamPose            output{};
    Lens               fixedLens{};
    vector<TrackState> tracks;
    vector<MixerGroup> groups;
};
struct TrackSpawnRequest
{
    CamTrackType      type     = CamTrackType::Sequence;
    CamPriority       priority = CamPriority::High;
    CamLayer          layer    = CamLayer::Action;
    AnchorBinding     anchor{};
    FollowTrackDesc   followDesc{};
    SequenceTrackDesc seqDesc{};
};
struct TrackKillRequest
{
    TrackID id{};
    bool    immediate = false; 
    float   fadeOutSec = 0.f;
};

NS_END