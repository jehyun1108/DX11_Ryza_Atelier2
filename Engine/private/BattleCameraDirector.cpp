#include "Enginepch.h"
// ==============================================================
static inline float HalfLifeAlpha(float hl, float dt)
{
    if (hl <= 0.f) return 1.f;
    const float k = 0.69314718056f; // ln(2)
    return 1.f - expf(-k * dt / hl);
}
static inline float QuatAngleRad(_vec a, _vec b)
{
    float dot = XMVectorGetX(XMVector4Dot(a, b));
    dot = clamp(dot, -1.f, 1.f);
    return 2.f * acosf(fabsf(dot));
}
BattleCameraDirector::BattleCameraDirector(SystemRegistry& registry) : registry(registry)
{
    state.fixedLens.fovY   = 30.f;  
    state.fixedLens.nearZ  = 0.1f;
    state.fixedLens.farZ   = 5000.f;

    state.output.lens      = state.fixedLens;

    smooth.halfLifePosSec  = 0.15f; 
    smooth.halfLifeRotSec  = 0.15f; 
}
void BattleCameraDirector::OnBoot()
{
    timelineSys = &registry.Get<BattleTimelineSystem>();
    targetSys   = &registry.Get<BattleTargetSystem>();
    tfSys       = &registry.Get<TransformSystem>();
    camSys      = &registry.Get<CameraSystem>();
}

void BattleCameraDirector::BindCam(Handle camHandle)
{
    cam                   = camHandle;
    auto* camData         = camSys->Get(cam);
    auto* tf              = tfSys->Get(camData->transform);

    state.output.pos      = tf->pos;
    state.output.rot      = tf->rot;

    state.fixedLens.nearZ = camData->nearZ;
    state.fixedLens.farZ  = camData->farZ;
    state.output.lens     = state.fixedLens;

    camSys->SetPerspective(cam, state.fixedLens.fovY, camData->aspect, state.fixedLens.nearZ, state.fixedLens.farZ);
}

void BattleCameraDirector::SetFixedLens(const Lens& lens)
{
    state.fixedLens = lens;
    state.output.lens = lens;
}

TrackID BattleCameraDirector::Spawn(const TrackSpawnRequest& req)
{
    TrackState track{};
    track.type               = req.type;
    track.priority           = req.priority;
    track.layer              = req.layer;
    track.anchor             = req.anchor;
    track.followDesc         = req.followDesc;
    track.seqDesc            = req.seqDesc;
    track.seqRt.lockUntilEnd = req.seqDesc.lockUntilEnd;
    track.seqRt.loop         = req.seqDesc.loop;
    track.seqRt.timeScale    = req.seqDesc.timeScale;

    track.valid              = true;
    track.id                 = AllocID();
    track.goal               = state.output;

    state.tracks.push_back(track);
    RebuildGroups();
    return track.id;
}

void BattleCameraDirector::Kill(const TrackKillRequest& req)
{
    TrackState& track = RequireTrack(req.id);
    if (req.immediate || req.fadeOutSec <= 0.f)
    {
        track.valid = false;
        RebuildGroups();
        return;
    }
    track.dying = true;
    track.fadeOutSec = req.fadeOutSec;
    track.fadeOutRemain = req.fadeOutSec;
}

void BattleCameraDirector::SetAnchor(TrackID id, const AnchorBinding& anchor)
{
    TrackState& track = RequireTrack(id);
    track.anchor = anchor;
}

void BattleCameraDirector::SetPriority(TrackID id, CamPriority priority)
{
    TrackState& track = RequireTrack(id);
    track.priority = priority;
    RebuildGroups();
}

void BattleCameraDirector::SetLayer(TrackID id, CamLayer layer)
{
    TrackState& track = RequireTrack(id);
    track.layer = layer;
}

void BattleCameraDirector::SetGoal(TrackID id, const CamPose& goal)
{
    TrackState& track = RequireTrack(id);
    track.goal = goal;
}

void BattleCameraDirector::SetSeqClips(TrackID id, const vector<ShotClip>& clips)
{
    TrackState& track = RequireTrack(id);
    track.seqRt.clips = clips;
    track.seqRt.curIdx = 0;
    track.seqRt.localTime = 0.0;
}

void BattleCameraDirector::SnapTrackToPose(TrackID id, const CamPose& pose)
{
    TrackState& t = RequireTrack(id);
    t.goal = pose;
}

void BattleCameraDirector::SetSeqDesc(TrackID id, const SequenceTrackDesc& desc)
{
    TrackState& track = RequireTrack(id);
    track.seqDesc = desc;
    track.seqRt.lockUntilEnd = desc.lockUntilEnd;
    track.seqRt.loop = desc.loop;
    track.seqRt.timeScale = desc.timeScale;
}

void BattleCameraDirector::SetDebugCam(Handle tf)
{
    debugCamTf = tf;
    debugCamActive = tf.IsValid();
}

void BattleCameraDirector::ClearDebugCam()
{
    debugCamActive = false;
}

void BattleCameraDirector::RebuildGroups()
{
    state.groups.clear();

    struct TempGroup
    {
        CamPriority  priority;
        vector<_uint> indices;
    };

    vector<TempGroup> temp;

    for (_uint i = 0; i < (_uint)state.tracks.size(); ++i)
    {
        auto& track = state.tracks[i];
        if (!track.valid) continue;

        bool found = false;
        for (auto& g : temp)
        {
            if (g.priority == track.priority)
            {
                g.indices.push_back(i);
                found = true;
                break;
            }
        }
        if (!found)
        {
            TempGroup g{};
            g.priority = track.priority;
            g.indices.push_back(i);
            temp.push_back(g);
        }
    }

    for (auto& g : temp)
    {
        MixerGroup mg{};
        mg.priority = g.priority;
        mg.trackIndices = g.indices;
        state.groups.push_back(mg);
    }

    sort(state.groups.begin(), state.groups.end(), [](const MixerGroup& a, const MixerGroup& b) {  return (int)a.priority > (int)b.priority; });
}

void BattleCameraDirector::AdvanceTracks(float dt)
{
    if (debugCamActive && debugCamTf.IsValid())
    {
        const _float4x4* w = tfSys->GetWorld(debugCamTf);
        _mat m = XMLoadFloat4x4(w);

        CamPose pose{};
        pose.pos = _float3{ w->_41, w->_42, w->_43 };

        _vec q = XMQuaternionRotationMatrix(m);
        XMStoreFloat4(&pose.rot, q);

        Lens lens{};
        lens.fovY = XMConvertToRadians(60.f);
        pose.lens = lens;

        state.output = pose;
        return;
    }

    for (auto& track : state.tracks)
    {
        if (!track.valid)
            continue;

        // ★ 여기 추가: 죽이는 타이머 처리
        if (track.dying)
        {
            track.fadeOutRemain -= dt;
            if (track.fadeOutRemain <= 0.f)
            {
                track.valid = false;
                continue; // 이 트랙은 더 이상 업데이트 안 함
            }
        }

        switch (track.type)
        {
        case CamTrackType::Follow:
            AdvanceFollow(track, dt);
            break;

        case CamTrackType::Sequence:
            AdvanceSequence(track, dt);
            break;

        case CamTrackType::Scripted:
            break;

        case CamTrackType::Shake:
            break;
        }
    }

    state.tracks.erase(
        remove_if(
            state.tracks.begin(),
            state.tracks.end(),
            [](const TrackState& t) { return !t.valid; }),
        state.tracks.end());

    RebuildGroups();
}

void BattleCameraDirector::ApplyAnchors(TrackState& track, const CamPose* localOpt)
{
    _vec basePos = XMVectorZero();
    _vec baseRot = XMQuaternionIdentity();

    if (track.anchor.space == AnchorSpace::World)
    {
        basePos = XMVectorZero();
        baseRot = XMQuaternionIdentity();
    }
    else
    {
        EntityID entity = ResolveAnchorEntity(track.anchor);
        if (!GetEntityWorldPos(entity, basePos, baseRot))
        {
            basePos = XMVectorZero();
            baseRot = XMQuaternionIdentity();
        }
    }

    const _mat mOffset = (track.anchor.space == AnchorSpace::Target)? XMMatrixRotationQuaternion(baseRot) : XMMatrixIdentity();

    if (localOpt)
    {
        _vec localPos = XMLoadFloat3(&localOpt->pos);
        _vec localRot = XMLoadFloat4(&localOpt->rot);
        _vec worldPos = basePos + XMVector3TransformNormal(localPos, mOffset);
        _vec worldRot = (track.anchor.space == AnchorSpace::Target) ? XMQuaternionNormalize(XMQuaternionMultiply(baseRot, localRot)):XMQuaternionNormalize(localRot);

        XMStoreFloat3(&track.goal.pos, worldPos);
        XMStoreFloat4(&track.goal.rot, worldRot);
        track.goal.lens = state.fixedLens;
        return;
    }

    _vec vOffset = XMLoadFloat3(&track.anchor.offset);
    _vec vWorldOffset = XMVector3TransformNormal(vOffset, mOffset);
    XMStoreFloat3(&track.goal.pos, basePos + vWorldOffset);
    XMStoreFloat4(&track.goal.rot, XMQuaternionNormalize(baseRot));
    track.goal.lens = state.fixedLens;
}

void BattleCameraDirector::AdvanceFollow(TrackState& track, float dt)
{
    EntityID leader = timelineSys->GetLeader();
    if (leader == 0u)
        return;

    EntityID target = targetSys->Get(leader);

    Handle        leaderTf{};
    TransformData* leaderData = tfSys->GetByOwner(leader, &leaderTf);
    if (!leaderData)
        return;

    _float3 leaderPos = leaderData->pos;
    _float3 targetPos = leaderPos;

    if (target != 0u)
    {
        Handle        targetTf{};
        TransformData* targetData = tfSys->GetByOwner(target, &targetTf);
        if (targetData)
            targetPos = targetData->pos;
    }

    // 리더/타겟 중간점 (포커스)
    const float alpha = 0.6f;
    _float3 focus{};
    focus.x = leaderPos.x + (targetPos.x - leaderPos.x) * alpha;
    focus.y = leaderPos.y + (targetPos.y - leaderPos.y) * alpha;
    focus.z = leaderPos.z + (targetPos.z - leaderPos.z) * alpha;

    // 궤도 중심 (리더 위치 + 높이)
    _float3 center{};
    center.x = leaderPos.x;
    center.y = leaderPos.y + track.followDesc.orbitHeight;
    center.z = leaderPos.z;

    // ----- 중심/포커스 보간 -----
    // half-life 는 기존 카메라 스무딩과 비슷하게 사용 (원하면 따로 값 뺄 수 있음)
    float centerAlpha = HalfLifeAlpha(smooth.halfLifePosSec, dt);

    if (!track.hasFollowHistory)
    {
        track.followCenter = center;
        track.followFocus = focus;
        track.hasFollowHistory = true;
    }
    else
    {
        track.followCenter.x += (center.x - track.followCenter.x) * centerAlpha;
        track.followCenter.y += (center.y - track.followCenter.y) * centerAlpha;
        track.followCenter.z += (center.z - track.followCenter.z) * centerAlpha;

        track.followFocus.x += (focus.x - track.followFocus.x) * centerAlpha;
        track.followFocus.y += (focus.y - track.followFocus.y) * centerAlpha;
        track.followFocus.z += (focus.z - track.followFocus.z) * centerAlpha;
    }

    // 여기부터는 "보간된" 중심/포커스를 기준으로 궤도/시선 계산
    _vec up = Utility::Up();

    float dx = track.followFocus.x - track.followCenter.x;
    float dz = track.followFocus.z - track.followCenter.z;

    _vec toTargetXZ = XMVectorSet(dx, 0.f, dz, 0.f);
    float lenSq = XMVectorGetX(XMVector3LengthSq(toTargetXZ));
    _vec f{};

    if (lenSq < 1e-4f)
    {
        // 타겟과 너무 가까우면 리더의 Forward XZ 를 사용
        _float2 forwardXZ = tfSys->GetForwardXZ(leaderTf);
        f = XMVector3Normalize(XMVectorSet(forwardXZ.x, 0.f, forwardXZ.y, 0.f));
    }
    else
        f = XMVector3Normalize(toTargetXZ);

    _vec r = XMVector3Normalize(XMVector3Cross(up, f));

    const float radius = max(0.f, track.followDesc.orbitRadius);
    const float yawDeg = track.followDesc.yawOffsetDeg;
    const float yawRad = XMConvertToRadians(yawDeg);

    _vec centerV = XMVectorSet(track.followCenter.x, track.followCenter.y, track.followCenter.z, 0.f);

    _vec dir = XMVector3Normalize(
        XMVectorScale(-f, cosf(yawRad)) +
        XMVectorScale(r, sinf(yawRad)));

    _vec posV = centerV + XMVectorScale(dir, radius);
    XMStoreFloat3(&track.goal.pos, posV);

    _vec aimV = XMVectorSet(
        track.followFocus.x,
        track.followFocus.y + track.followDesc.aimOffsetY,
        track.followFocus.z,
        0.f);

    _vec lookDir = XMVector3Normalize(aimV - posV);
    _vec quat = Utility::BuildLookRot(lookDir, up);
    XMStoreFloat4(&track.goal.rot, XMQuaternionNormalize(quat));

    track.goal.lens = state.fixedLens;
}

void BattleCameraDirector::AdvanceSequence(TrackState& track, float dt)
{
    track.seqRt.localTime += static_cast<double>(dt * track.seqRt.timeScale);

    if (track.seqRt.clips.empty())
    {
        ApplyAnchors(track);

        _vec camPos = XMLoadFloat3(&track.goal.pos);
        _vec targetPos = XMVectorZero();

        if (ComputeFollowTarget(track, targetPos))
        {
            _vec forward = XMVector3Normalize(targetPos - camPos);
            _vec up = Utility::Up();
            _vec quat = Utility::BuildLookRot(forward, up);
            XMStoreFloat4(&track.goal.rot, XMQuaternionNormalize(quat));
        }

        track.goal.lens = state.fixedLens;
        return;
    }

    size_t n = track.seqRt.clips.size();
    size_t i = track.seqRt.curIdx;
    auto* clip = &track.seqRt.clips[i];

    while (track.seqRt.localTime > clip->t1)
    {
        double over = track.seqRt.localTime - clip->t1;

        if (i + 1 < n)
        {
            ++i;
            clip = &track.seqRt.clips[i];
            track.seqRt.localTime = clip->t0 + over;
        }
        else if (track.seqRt.loop)
        {
            i = 0;
            clip = &track.seqRt.clips[i];
            double span = max(1e-5, (clip->t1 - clip->t0));
            track.seqRt.localTime = clip->t0 + fmod(over, span);
        }
        else
        {
            track.seqRt.localTime = clip->t1;

            if (!track.seqRt.lockUntilEnd)
                track.valid = false;

            break;
        }
    }

    track.seqRt.curIdx = i;

    if (!track.valid)
        return;

    bool sampled = false;

    if (seqSampler)
    {
        CamPose sampledPose{};
        double  seqTime = track.seqRt.localTime;

        if (seqSampler(track.seqDesc.clipId, seqTime, track.seqDesc, sampledPose))
        {
            track.goal = sampledPose;
            sampled = true;
        }
    }

    if (!sampled)
    {
        ApplyAnchors(track);

        _vec camPos = XMLoadFloat3(&track.goal.pos);
        _vec targetPos = XMVectorZero();

        if (ComputeFollowTarget(track, targetPos))
        {
            _vec forward = XMVector3Normalize(targetPos - camPos);
            _vec up = Utility::Up();
            _vec quat = Utility::BuildLookRot(forward, up);
            XMStoreFloat4(&track.goal.rot, XMQuaternionNormalize(quat));
        }
        track.goal.lens = state.fixedLens;
    }
}

CamPose BattleCameraDirector::MixLayered(const vector<const TrackState*>& base, const vector<const TrackState*>& action, const vector<const TrackState*>& overlay) const
{
    const TrackState* chosen = nullptr;

    if (!overlay.empty())
        chosen = overlay.back();   
    else if (!action.empty())
        chosen = action.back();   
    else if (!base.empty())
        chosen = base.back();      

    if (!chosen)
        return state.output;       
    CamPose out = chosen->goal;
    out.lens = state.fixedLens;
    return out;
}

CamPose BattleCameraDirector::MixByGroups(const CamPose& prev) const
{
    if (state.groups.empty())
        return state.output;
    const MixerGroup& g = state.groups.front();
    return MixGroup(g);
}

CamPose BattleCameraDirector::MixGroup(const MixerGroup& group) const
{
    vector<const TrackState*> base, action, overlay;
    base.reserve(group.trackIndices.size());
    action.reserve(group.trackIndices.size());
    overlay.reserve(group.trackIndices.size());

    for (auto idx : group.trackIndices)
    {
        const TrackState& t = state.tracks[idx];
        if (!t.valid) continue;

        if (t.layer == CamLayer::Base)        base.push_back(&t);
        else if (t.layer == CamLayer::Action) action.push_back(&t);
        else                                  overlay.push_back(&t);
    }

    return MixLayered(base, action, overlay);
}

TrackState* BattleCameraDirector::Find(TrackID id)
{
    if (!id.IsValid()) return nullptr;
    for (auto& t : state.tracks) if (t.id.idx == id.idx && t.id.gen == id.gen) return &t;
    return nullptr;
}

const TrackState* BattleCameraDirector::Find(TrackID id) const
{
    if (!id.IsValid()) return nullptr;
    for (auto& t : state.tracks) if (t.id.idx == id.idx && t.id.gen == id.gen) return &t;
    return nullptr;
}

TrackID BattleCameraDirector::AllocID()
{
    const _uint idx = nextIdx++;
    const _uint g = ++gen[idx];
    return TrackID{ idx, g };
}

EntityID BattleCameraDirector::ResolveAnchorEntity(const AnchorBinding& anchor) const
{
    switch (anchor.binding)
    {
    case TargetBinding::None:      
        break;

    case TargetBinding::Leader:
    {
        if (anchor.entity != 0u)
            return anchor.entity;
        return timelineSys->GetLeader();
    }

    case TargetBinding::CurTarget: 
    { 
        EntityID owner = anchor.entity;
        if (owner == 0u)
            owner = timelineSys->GetLeader();

        if (owner == 0u)
            return 0u;

        return targetSys->Get(owner);
    }
    case TargetBinding::CustomEntity: 
        return anchor.entity;
    }
    return 0;
}

bool BattleCameraDirector::GetEntityWorldPos(EntityID entity, _vec& outPos, _vec& outRot) const
{
    if (entity == 0u)
        return false;

    Handle tfHandle{};
    TransformData* tf = tfSys->GetByOwner(entity, &tfHandle);
    if (!tf)
        return false;

    outPos = XMLoadFloat3(&tf->pos);
    outRot = XMQuaternionNormalize(XMLoadFloat4(&tf->rot));
    return true;
}

bool BattleCameraDirector::ComputeFollowTarget(const TrackState& track, _vec& outTargetPos) const
{
    if (track.anchor.binding == TargetBinding::None)
        return false;

    EntityID entity = ResolveAnchorEntity(track.anchor);
    if (entity == 0u)
        return false;

    _vec pos = XMVectorZero();
    _vec quat = XMQuaternionIdentity();
    if (!GetEntityWorldPos(entity, pos, quat))
        return false;

    outTargetPos = pos + XMVectorSet(0.f, 1.6f, 0.f, 0.f);
    return true;
}

TrackState& BattleCameraDirector::RequireTrack(TrackID id)
{
    assert(id.IsValid());
    for (auto& track : state.tracks)
    {
        if (track.id.idx == id.idx && track.id.gen == id.gen)
            return track;
    }
    assert(false);
    return state.tracks[0];
}

void BattleCameraDirector::SnapTrackToOutput(TrackID id)
{
    TrackState& t = RequireTrack(id);
    t.goal = state.output;
}

void BattleCameraDirector::Tick(float dt)
{
    AdvanceTracks(dt);

    CamPose target = MixByGroups(state.output);

    Lens desiredLens = state.fixedLens;
    if (HasActiveSequenceTrack())
    {
        Lens seqLens{};
        if (GetTopSequenceLens(seqLens))
            desiredLens = seqLens;
    }

    auto halfLife = [](float hl, float dt)
        {
            if (hl <= 0.f) return 1.f;
            const float k = 0.69314718056f; 
            return 1.f - expf(-k * dt / hl);
        };

    float ap = halfLife(smooth.halfLifePosSec, dt);
    float ar = halfLife(smooth.halfLifeRotSec, dt);

    _vec p0 = XMLoadFloat3(&state.output.pos);
    _vec p1 = XMLoadFloat3(&target.pos);
    _vec np = XMVectorLerp(p0, p1, ap);
    XMStoreFloat3(&state.output.pos, np);

    _vec q0 = XMLoadFloat4(&state.output.rot);
    _vec q1 = XMLoadFloat4(&target.rot);
    _vec nq = XMQuaternionSlerp(q0, q1, ar);
    XMStoreFloat4(&state.output.rot, XMQuaternionNormalize(nq));

    float af = ar; 

    if (state.output.lens.fovY <= 0.f)
        state.output.lens.fovY = desiredLens.fovY;
    else
        state.output.lens.fovY = state.output.lens.fovY + (desiredLens.fovY - state.output.lens.fovY) * af;

    state.output.lens.nearZ = desiredLens.nearZ;
    state.output.lens.farZ = desiredLens.farZ;

    if (auto* c = camSys->Get(cam))
    {
        if (auto* td = tfSys->Get(c->transform))
        {
            XMStoreFloat3(&td->pos, XMLoadFloat3(&state.output.pos));
            XMStoreFloat4(&td->rot, XMQuaternionNormalize(XMLoadFloat4(&state.output.rot)));
            td->dirty = true;
        }
        camSys->SetPerspective(  cam, state.output.lens.fovY,  c->aspect,   state.output.lens.nearZ,  state.output.lens.farZ  );
    }
}

FollowTrackDesc& BattleCameraDirector::GetFollowDesc(TrackID id)
{
    TrackState& track = RequireTrack(id);
    assert(track.type == CamTrackType::Follow);
    return track.followDesc;
}

bool BattleCameraDirector::HasActiveSequenceTrack() const
{
    for (const auto& t : state.tracks)
    {
        if (!t.valid) continue;
        if (t.type != CamTrackType::Sequence) continue;
        return true;
    }
    return false;
}

bool BattleCameraDirector::GetTopSequenceLens(Lens& outLens) const
{
    const TrackState* best = {};

    for (const auto& t : state.tracks)
    {
        if (!t.valid) continue;
        if (t.type != CamTrackType::Sequence) continue;

        if (!best || (int)t.priority > (int)best->priority)
            best = &t;
    }

    if (!best) return false;

    outLens = best->goal.lens;
    return true;
}