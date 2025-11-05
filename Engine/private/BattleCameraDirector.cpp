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
// ===========================================================================================

BattleCameraDirector::BattleCameraDirector(SystemRegistry& registry) : registry(registry)
{
    state.fixedLens   = Lens{};
    state.output.lens = state.fixedLens;
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
    track.phase              = TrackPhase::FadingIn;
    track.activeW            = 0.f;
    track.valid              = true;
    track.id                 = AllocID();
    track.cur                = state.output;
    track.cur.lens           = state.fixedLens;
    track.goal               = track.cur;
    state.tracks.push_back(track);
    RebuildGroups();
    return track.id;
}

bool BattleCameraDirector::Kill(const TrackKillRequest& req)
{
    TrackState* track = Find(req.id);
    if (!track) return false;
    if (req.immediate)
    {
        track->phase   = TrackPhase::Inactive;
        track->activeW = 0.f;
        track->valid   = false;
    }
    else
        track->phase = TrackPhase::FadingOut;

    RebuildGroups();
    return true;
}

bool BattleCameraDirector::SetAnchor(TrackID id, const AnchorBinding& anchor)
{
    if (auto track = Find(id))
    {
        track->anchor = anchor;
        return true;
    }
    return false;
}

bool BattleCameraDirector::SetPriority(TrackID id, CamPriority priority)
{
    if (auto track = Find(id))
    {
        track->priority = priority;
        return true;
    }
    return false;
}

bool BattleCameraDirector::SetLayer(TrackID id, CamLayer layer)
{
    if (auto track = Find(id))
    {
        track->layer = layer;
        return true;
    }
    return false;
}

bool BattleCameraDirector::SetGoal(TrackID id, const CamPose& goal)
{
    if (auto track = Find(id))
    {
        track->goal = goal;
        return true;
    }
    return false;
}

bool BattleCameraDirector::SetSeqClips(TrackID id, const vector<ShotClip>& clips)
{
    if (auto track = Find(id))
    {
        track->seqRt.clips     = clips;
        track->seqRt.curIdx    = 0;
        track->seqRt.localTime = 0.0;
        return true;
    }
    return false;
}

void BattleCameraDirector::RebuildGroups()
{
    state.groups.clear();
    struct Temp { CamPriority priority; vector<_uint> idx; }; 
    vector<Temp> temp;
    
    for (_uint i = 0; i < (_uint)state.tracks.size(); ++i)
    {
        auto& track = state.tracks[i];
        if (!track.valid) continue;
        bool found = false;
        for (auto& group : temp)
        {
            if (group.priority == track.priority)
            {
                group.idx.push_back(i);
                found = true;
                break;
            }
        }
        if (!found)
            temp.push_back(Temp{ track.priority, { i } });
    }

    for (auto& group : temp)
    {
        MixerGroup mGroup{};
        mGroup.priority     = group.priority;
        mGroup.trackIndices = group.idx;
        state.groups.push_back(mGroup);
    }
    sort(state.groups.begin(), state.groups.end(), [](const MixerGroup& a, const MixerGroup& b) { return (int)a.priority > (int)b.priority; });
}

void BattleCameraDirector::AdvanceTracks(float dt)
{
    state.elapsed += dt;
    for (auto& track : state.tracks)
    {
        if (!track.valid) continue;
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

        ApplyTrackSmoothing(track, dt);
        SnapIfClose(track);
        if (track.phase == TrackPhase::FadingIn)
        {
            track.activeW += HalfLifeAlpha(smooth.halfLifePosSec, dt);
            if (track.activeW >= 1.f)
            {
                track.activeW = 1.f;
                track.phase   = TrackPhase::Sustaining;
            }
        }
        else if (track.phase == TrackPhase::FadingOut)
        {
            track.activeW -= HalfLifeAlpha(smooth.halfLifePosSec, dt);
            if (track.activeW <= 0.f)
            {
                track.activeW = 0.f;
                track.phase   = TrackPhase::Inactive;
                track.valid   = false;
            }
        }
    }

    state.tracks.erase(remove_if(state.tracks.begin(), state.tracks.end(), [](const TrackState& track) { return !track.valid; }), state.tracks.end());
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
    const _mat mOffset = (track.anchor.space == AnchorSpace::Target) ? XMMatrixRotationQuaternion(baseRot) : XMMatrixIdentity();

    if (localOpt)
    {
        _vec localPos = XMLoadFloat3(&localOpt->pos);
        _vec localRot = XMLoadFloat4(&localOpt->rot);
        _vec worldPos = basePos + XMVector3TransformNormal(localPos, mOffset);
        _vec worldRot = (track.anchor.space == AnchorSpace::Target) ? XMQuaternionNormalize(XMQuaternionMultiply(baseRot, localRot)) 
                                                                    : XMQuaternionNormalize(localRot);
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
    ApplyAnchors(track);
    _vec vTargetPos = XMVectorZero();
    bool hasTarget = ComputeFollowTarget(track, vTargetPos);

    _vec up = Utility::Up();
    _float2 forwardXZ = { 0.f, 1.f };
    {
        EntityID entity = ResolveAnchorEntity(track.anchor);
        auto& tfSys = registry.Get<TransformSystem>();
        Handle tfHandle{};
        tfSys.GetByOwner(entity, &tfHandle);
        if (tfHandle.IsValid())
            forwardXZ = tfSys.GetForwardXZ(tfHandle);
        if (fabs(forwardXZ.x) + fabs(forwardXZ.y) < 1e-6f)
            forwardXZ = _float2(0.f, 1.f);
    }

    _vec vForward = XMVector3Normalize(XMVectorSet(forwardXZ.x, 0.f, forwardXZ.y, 0.f));
    _vec vRight   = XMVector3Normalize(XMVector3Cross(up, vForward));
    _vec vUp      = XMVector3Normalize(XMVector3Cross(vForward, vRight));

    const float radius = max(0.f, track.followDesc.orbitRadius);
    const float height = track.followDesc.orbitHeight;

    _vec tPos = XMVectorZero();
    _vec tRot = XMQuaternionIdentity();
    {
        EntityID entity = ResolveAnchorEntity(track.anchor);
        GetEntityWorldPos(entity, tPos, tRot);
    }
    _vec center = tPos + XMVectorScale(vUp, height);

    float azDeg = track.followDesc.yawOffsetDeg;
    float azRad = XMConvertToRadians(azDeg);
    _vec  dir   = XMVector3Normalize(XMVectorScale(vForward, -cosf(azRad)) + XMVectorScale(vRight, sinf(azRad)));
    _vec  pos   = center + XMVectorScale(dir, radius);
    XMStoreFloat3(&track.goal.pos, pos);

    if (hasTarget)
        vTargetPos = vTargetPos + XMVectorSet(0.f, track.followDesc.aimOffsetY, 0.f, 0.f);
    _vec forward = hasTarget ? XMVector3Normalize(vTargetPos - pos) : XMVector3Normalize(center - pos);
    _vec quat = Utility::BuildLookRot(forward, vUp);
    XMStoreFloat4(&track.goal.rot, XMQuaternionNormalize(quat));
    track.goal.lens = state.fixedLens;
}

void BattleCameraDirector::AdvanceSequence(TrackState& track, float dt)
{
    track.seqRt.localTime += static_cast<double>(dt * track.seqRt.timeScale);
    if (track.seqRt.clips.empty())
        ApplyAnchors(track);
    else
    {
        size_t n = track.seqRt.clips.size();
        size_t i = track.seqRt.curIdx;
        const ShotClip* clip = &track.seqRt.clips[i];
        while (track.seqRt.localTime > clip->t1)
        {
            double over = track.seqRt.localTime - clip->t1;
            if (i + 1 < n)
            {
                i++;
                clip = &track.seqRt.clips[i];
                track.seqRt.localTime = clip->t0 + over;
            }
            else if (track.seqRt.loop)
            {
                i = 0;
                clip = &track.seqRt.clips[i];
                track.seqRt.localTime = clip->t0 + fmod(over, max(1e-5, (clip->t1 - clip->t0)));
            }
            else
            {
                if (!track.seqRt.lockUntilEnd)
                {
                    track.phase = TrackPhase::FadingOut;
                    break;
                }
            }
        }
        track.seqRt.curIdx = i;
        if (seqSampler)
        {
            CamPose local{};
            double trackClip = track.seqRt.localTime - clip->t0;
            if (seqSampler(track.seqDesc.clipId, trackClip, track.seqDesc, local))
                ApplyAnchors(track, &local);
            else
                ApplyAnchors(track);
        }
        else
            ApplyAnchors(track);
    }

    _vec camPos = XMLoadFloat3(&track.goal.pos);
    _vec targetPos = XMVectorZero();
    bool hasTarget = ComputeFollowTarget(track, targetPos);

    if (hasTarget)
    {
        _vec forward = XMVector3Normalize(targetPos - camPos);
        _vec up = Utility::Up();
        _vec quat = Utility::BuildLookRot(forward, up);
        XMStoreFloat4(&track.goal.rot, XMQuaternionNormalize(quat));
    }
    track.goal.lens = state.fixedLens;
}

void BattleCameraDirector::ApplyTrackSmoothing(TrackState& track, float dt)
{
    float ap = HalfLifeAlpha(smooth.halfLifePosSec, dt);
    float ar = HalfLifeAlpha(smooth.halfLifeRotSec, dt);

    _vec cp = XMLoadFloat3(&track.cur.pos);
    _vec cg = XMLoadFloat3(&track.goal.pos);
    _vec np = XMVectorLerp(cp, cg, ap);
    XMStoreFloat3(&track.cur.pos, np);

    _vec cq = XMLoadFloat4(&track.cur.rot), gq = XMLoadFloat4(&track.goal.rot);
    _vec nq = XMQuaternionSlerp(cq, gq, ar);
    XMStoreFloat4(&track.cur.rot, XMQuaternionNormalize(nq));

    track.cur.lens = state.fixedLens;
}

void BattleCameraDirector::SnapIfClose(TrackState& track)
{
    _vec  curPos  = XMLoadFloat3(&track.cur.pos);
    _vec  goalPos = XMLoadFloat3(&track.goal.pos);
    float dist    = XMVectorGetX(XMVector3Length(goalPos - curPos));
    _vec  curRot  = XMLoadFloat4(&track.cur.rot);
    _vec  goalRot = XMLoadFloat4(&track.goal.rot);
    float ang     = QuatAngleRad(curRot, goalRot);
    if (dist < smooth.epsilonPos && XMConvertToDegrees(ang) < smooth.epsilonRotDeg)
        track.cur = track.goal;
}

CamPose BattleCameraDirector::ClampStep(const CamPose& prev, const CamPose& next, float dt) const
{
    CamPose o = next;

    _vec p0 = XMLoadFloat3(&prev.pos), p1 = XMLoadFloat3(&next.pos);
    float dist = XMVectorGetX(XMVector3Length(p1 - p0));
    float maxStepPos = smooth.maxPosMps * dt;
    if (dist > maxStepPos && dist > 1e-6f)
    {
        float t = maxStepPos / dist;
        XMStoreFloat3(&o.pos, XMVectorLerp(p0, p1, t));
    }

    _vec q0 = XMLoadFloat4(&prev.rot), q1 = XMLoadFloat4(&next.rot);
    float ang = QuatAngleRad(q0, q1);
    float maxStepRot = XMConvertToRadians(smooth.maxRotDegPerSec) * dt;
    if (ang > maxStepRot && ang > 1e-6f)
    {
        float t = maxStepRot / ang;
        _vec q = XMQuaternionSlerp(q0, q1, t);
        XMStoreFloat4(&o.rot, XMQuaternionNormalize(q));
    }

    o.lens = state.fixedLens;
    return o;
}

CamPose BattleCameraDirector::MixLayered(const vector<const TrackState*>& base, const vector<const TrackState*>& action, const vector<const TrackState*>& overlay) const
{
    auto pick = [](const std::vector<const TrackState*>& v)->const TrackState*
        {
            const TrackState* best = nullptr; float w = -1.f;
            for (auto* t : v) if (t && t->activeW > w) { w = t->activeW; best = t; }
            return best;
        };
    const TrackState* tb = pick(base);
    CamPose out = tb ? tb->cur : CamPose{};

    auto mix1 = [&out](const TrackState* t)
        {
            if (!t) return;
            float w = std::clamp(t->activeW, 0.f, 1.f);
            _vec p0 = XMLoadFloat3(&out.pos), q0 = XMLoadFloat4(&out.rot);
            _vec p1 = XMLoadFloat3(&t->cur.pos), q1 = XMLoadFloat4(&t->cur.rot);
            XMStoreFloat3(&out.pos, XMVectorLerp(p0, p1, w));
            XMStoreFloat4(&out.rot, XMQuaternionSlerp(q0, q1, w));
            out.lens = out.lens; // fixed
        };

    mix1(pick(action));
    mix1(pick(overlay));
    out.lens = state.fixedLens;
    return out;
}

CamPose BattleCameraDirector::MixByGroups(const CamPose& prev) const
{
    if (state.groups.empty()) return prev;
    CamPose out = prev;
    for (int gi = (int)state.groups.size() - 1; gi >= 0; --gi)
    {
        const MixerGroup& g = state.groups[gi];
        CamPose gp = MixGroup(g);
        float w = 0.f;
        for (auto idx : g.trackIndices) { const TrackState& t = state.tracks[idx]; if (t.valid) w = max(w, clamp(t.activeW, 0.f, 1.f)); }
        _vec p0 = XMLoadFloat3(&out.pos), q0 = XMLoadFloat4(&out.rot);
        _vec p1 = XMLoadFloat3(&gp.pos), q1 = XMLoadFloat4(&gp.rot);
        XMStoreFloat3(&out.pos, XMVectorLerp(p0, p1, w));
        XMStoreFloat4(&out.rot, XMQuaternionSlerp(q0, q1, w));
        out.lens = state.fixedLens;
    }
    return out;
}

CamPose BattleCameraDirector::MixGroup(const MixerGroup& group) const
{
    vector<const TrackState*> base, action, overlay;
    base.reserve(group.trackIndices.size());
    action.reserve(group.trackIndices.size());
    overlay.reserve(group.trackIndices.size());
    for (auto idx : group.trackIndices)
    {
        const TrackState& t = state.tracks[idx]; if (!t.valid) continue;
        if (t.layer == CamLayer::Base)   base.push_back(&t);
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
    auto& tl = registry.Get<BattleTimelineSystem>();
    auto& tg = registry.Get<BattleTargetSystem>();
    switch (anchor.binding)
    {
    case TargetBinding::None:        break;
    case TargetBinding::Leader:      return tl.GetLeader();
    case TargetBinding::CurTarget: { EntityID leader = tl.GetLeader(); return (leader != invalidEntity) ? tg.Get(leader) : 0; }
    case TargetBinding::Attacker:
    case TargetBinding::Victim:
    case TargetBinding::CustomEntity: return anchor.entity;
    }
    return 0;
}

bool BattleCameraDirector::GetEntityWorldPos(EntityID entity, _vec& outPos, _vec& outRot) const
{
    auto& tf = registry.Get<TransformSystem>();
    Handle h{}; tf.GetByOwner(entity, &h);
    const TransformData* d = tf.Get(h);
    if (!d) return false;
    outPos = XMLoadFloat3(&d->pos);
    outRot = XMQuaternionNormalize(XMLoadFloat4(&d->rot));
    return true;
}

bool BattleCameraDirector::ComputeFollowTarget(const TrackState& track, _vec& outTargetPos) const
{
    if (track.anchor.binding == TargetBinding::None) return false;
    _vec p = XMVectorZero(), q = XMQuaternionIdentity();
    EntityID e = ResolveAnchorEntity(track.anchor);
    if (!GetEntityWorldPos(e, p, q)) return false;
    outTargetPos = p + XMVectorSet(0.f, 1.6f, 0.f, 0.f);
    return true;
}

void BattleCameraDirector::Tick(float dt)
{
    CamPose prev = state.output;
    AdvanceTracks(dt);
    CamPose raw = MixByGroups(prev);
    state.output = ClampStep(prev, raw, dt);

    auto& camSys = registry.Get<CameraSystem>();
    if (auto* c = camSys.Get(cam))
    {
        auto& tf = registry.Get<TransformSystem>();
        if (auto* td = tf.Get(c->transform))
        {
            XMStoreFloat3(&td->pos, XMLoadFloat3(&state.output.pos));
            XMStoreFloat4(&td->rot, XMQuaternionNormalize(XMLoadFloat4(&state.output.rot)));
            td->dirty = true;
        }
        camSys.SetPerspective(cam, state.fixedLens.fovY, c->aspect, state.fixedLens.nearZ, state.fixedLens.farZ);
    }
}