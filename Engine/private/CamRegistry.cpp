#include "Enginepch.h"

// =================================================================================================================
void CamRegistry::BindDirector(BattleCameraDirector& d)
{
    director = &d;
    if (!director) return;

    director->SetSequenceSampler([this](ClipId clip, double tLocal, const SequenceTrackDesc& desc, CamPose& outLocal)
        {
            auto it = samplers.find(clip);
            if (it == samplers.end()) return false;
            return it->second(clip, tLocal, desc, outLocal);
        });
}

void CamRegistry::BindCam(Handle cam)
{
	if (director)
		director->BindCam(cam);
}

void CamRegistry::SetSequenceSampler(BattleCameraDirector::SeqSampleFunc func)
{
	if (director)
		director->SetSequenceSampler(move(func));
}

void CamRegistry::RegisterDefaults()
{
    {
        TrackPreset follow{};
        follow.type                    = CamTrackType::Follow;
        follow.priority                = CamPriority::Default;
        follow.layer                   = CamLayer::Base;
        follow.anchor.space            = AnchorSpace::Target;
        follow.anchor.binding          = TargetBinding::Leader;
        follow.anchor.offset           = _float3{ 0.f, 150.f, 200.f };
        follow.followDesc.orbitRadius  = 200.f;  // ¾À    250~350
        follow.followDesc.orbitHeight  = 150.f;  // Ä³¸¯ÅÍ 160~220
        follow.followDesc.yawOffsetDeg = 0.f;    
        follow.followDesc.aimOffsetY   = 50.f;
        presets.emplace((int)BattleCamKey::Default_Follow, follow);
    }
}

TrackID CamRegistry::SpawnDefaultToFollow()
{
    TrackSpawnRequest req{};
    if (!Build(BattleCamKey::Default_Follow, req)) return {};
    const TrackID id = Spawn(req, false);
    if (!id.IsValid()) return {};
    baseFollowId = id;
    if (director)
    { 
        CamPose now = director->GetOutput();
        director->SetGoal(baseFollowId, now);
    }
    return id;
}

TrackID CamRegistry::SpawnIntro()
{
    TrackSpawnRequest req{};
    if (!Build(BattleCamKey::Intro_Action, req)) return {};
    TrackID id = Spawn(req, true);
    if (id.IsValid() && director)
    {
        auto it = introClips.find(req.seqDesc.clipId);
        if (it != introClips.end()) director->SetSeqClips(id, it->second);
    }
    return id;
}

void CamRegistry::KillRecent(float outDur)
{
	if (!director) return;
    if (baseFollowId.IsValid())
    {
        CamPose now = director->GetOutput();
        director->SetGoal(baseFollowId, now);
    }

    for (auto it = recent.rbegin(); it != recent.rend(); ++it)
    {
        TrackKillRequest kill{};
        kill.id = *it;
        director->Kill(kill);
    }
    recent.clear();
}

bool CamRegistry::Build(BattleCamKey key, TrackSpawnRequest& out) const
{
	auto it = presets.find((int)key);
	if (it == presets.end()) return false;
	const TrackPreset& preset = it->second;
	out.type       = preset.type;
	out.priority   = preset.priority;
	out.layer      = preset.layer;
	out.anchor     = preset.anchor;
	out.followDesc = preset.followDesc;
	out.seqDesc    = preset.seqDesc;
	return true;
}

TrackID CamRegistry::Spawn(const TrackSpawnRequest& req, bool trackRecent)
{
    if (!director) return {};

    TrackID id = director->Spawn(req);
    if (!id.IsValid()) return {};

    if (req.type == CamTrackType::Follow)
        baseFollowId = id;

    if (trackRecent)
        recent.push_back(id);

    return id;
}