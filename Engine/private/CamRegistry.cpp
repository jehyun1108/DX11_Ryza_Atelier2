#include "Enginepch.h"

#include "CamSerializer.h"
#include "ScreenFadeSystem.h"

void CamRegistry::OnBoot()
{
    director      = &registry.Get<BattleCameraDirector>();
    camSys        = &registry.Get<CameraSystem>();
    tfSys         = &registry.Get<TransformSystem>();
    camSerializer = &registry.Get<CamSerializer>();
    targetSys     = &registry.Get<BattleTargetSystem>();
    timelineSys   = &registry.Get<BattleTimelineSystem>();
    actionCamReg  = &registry.Get<ActionCamRegistry>();
    fadeSys       = &registry.Get<ScreenFadeSystem>();

    Lens lens{};
    lens.fovY = XMConvertToRadians(60.f);
    director->SetFixedLens(lens);

    RegisterDefaults();
    BindDirector();
}
void CamRegistry::BindDirector()
{
    director->SetSequenceSampler([this](ClipId clip, double tLocal, const SequenceTrackDesc& desc, CamPose& outLocal)
        {
            auto it = samplers.find(clip);
            if (it == samplers.end()) return false;
            return it->second(clip, tLocal, desc, outLocal);
        });
}

TrackID CamRegistry::SpawnDefaultToFollow()
{
    TrackSpawnRequest req{};
    if (!Build(BattleCamKey::Default_Follow, req)) return {};
    const TrackID id = Spawn(req, false);
    if (!id.IsValid()) return {};
    baseFollowId = id;
    CamPose now = director->GetOutput();
    director->SetGoal(baseFollowId, now);
    return id;
}

TrackID CamRegistry::SpawnIntro()
{
    TrackSpawnRequest req{};
    if (!Build(BattleCamKey::Intro_Action, req))
        return {};

    // 인트로는 recent 로 추적하지 않음
    TrackID id = Spawn(req, false);
    if (id.IsValid())
    {
        auto it = introClips.find(req.seqDesc.clipId);
        if (it != introClips.end())
            director->SetSeqClips(id, it->second);
    }
    return id;
}

void CamRegistry::KillRecent(float outDur)
{
	if (!director) return;
    if (baseFollowId.IsValid())
        director->SnapTrackToOutput(baseFollowId);

    for (auto it = recent.rbegin(); it != recent.rend(); ++it)
    {
        TrackKillRequest kill{};
        kill.id = *it;
        kill.immediate = false;
        kill.fadeOutSec = outDur;   
        director->Kill(kill);
    }
    recent.clear();
}

bool CamRegistry::Build(BattleCamKey key, TrackSpawnRequest& out) const
{
    auto it = presets.find((int)key);
    if (it == presets.end()) return false;
    out = it->second; 
    return true;
}

void CamRegistry::EvalSeqEvent(ClipId clip, const SeqCamPreset& preset, float t, CamPose& outLocal) const
{
    float dur = ComputeCamDuration(preset);
    if (t < 0.f) t = 0.f;
    if (t > dur) t = dur;

    const auto& keys = preset.keys;
    assert(!keys.empty());

    _float3 offPos{};
    _float3 offLook{};
    float   fovDeg = 60.f;

    if (keys.size() == 1)
    {
        offPos = keys[0].pos;
        offLook = keys[0].look;
        fovDeg = keys[0].fovDeg;
    }
    else if (t <= keys.front().t)
    {
        offPos = keys.front().pos;
        offLook = keys.front().look;
        fovDeg = keys.front().fovDeg;
    }
    else if (t >= keys.back().t)
    {
        offPos = keys.back().pos;
        offLook = keys.back().look;
        fovDeg = keys.back().fovDeg;
    }
    else
    {
        size_t seg = 0;
        for (size_t i = 0; i + 1 < keys.size(); ++i)
        {
            if (t >= keys[i].t && t <= keys[i + 1].t)
            {
                seg = i;
                break;
            }
        }

        const size_t n = keys.size();

        size_t i1 = seg;
        size_t i2 = seg + 1;
        size_t i0 = (i1 == 0) ? i1 : (i1 - 1);
        size_t i3 = (i2 + 1 >= n) ? i2 : (i2 + 1);

        const CamKey& k0 = keys[i0];
        const CamKey& k1 = keys[i1];
        const CamKey& k2 = keys[i2];
        const CamKey& k3 = keys[i3];

        float span = k2.t - k1.t;
        if (span <= 1e-5f) span = 1e-5f;

        float u = (t - k1.t) / span;
        u = EaseInOut(u);

        offPos = EvalCatmullRom(k0.pos, k1.pos, k2.pos, k3.pos, u);
        offLook = EvalCatmullRom(k0.look, k1.look, k2.look, k3.look, u);

        fovDeg = k1.fovDeg + (k2.fovDeg - k1.fovDeg) * u;
    }

    float shakeX = 0.f;
    float shakeY = 0.f;

    for (const CamShakeEvent& s : preset.shakes)
    {
        float half = s.width * 0.5f;
        float local = t - s.t;

        if (local < -half || local > half)
            continue;

        float v = (local + half) / (2.f * half);
        float env = 1.f - fabsf(v * 2.f - 1.f);

        float phase = (local / s.width) * XM_PI;
        float osc = sinf(phase);

        float amp = s.amp * env * osc;

        shakeX += amp;
        shakeY += amp * s.yScale;
    }

    offPos.x += shakeX;
    offPos.y += shakeY;

    _float3 leaderPos{};
    _float3 focusPos{};
    _vec    right{}, up{}, fwd{};

    EntityID basisEntity = 0u;

    auto itBasis = seqBasisEntity.find(clip);
    if (itBasis != seqBasisEntity.end())
        basisEntity = itBasis->second;
    else
        basisEntity = timelineSys->GetLeader();

    if (!ComputeFocusPosFor(basisEntity, leaderPos, focusPos, right, up, fwd))
    {
        outLocal.pos = _float3{ 0.f, 0.f, -500.f };

        _vec lookDir = XMVectorSet(0.f, 0.f, 1.f, 0.f);
        _vec q = Utility::BuildLookRot(lookDir, Utility::Up());
        XMStoreFloat4(&outLocal.rot, q);

        Lens lens{};
        lens.fovY = XMConvertToRadians(fovDeg);
        lens.nearZ = 0.1f;
        lens.farZ = 5000.f;
        outLocal.lens = lens;
        return;
    }

    _vec leaderV = XMLoadFloat3(&leaderPos);

    _vec camV =
        leaderV
        + XMVectorScale(right, offPos.x)
        + XMVectorScale(up, offPos.y)
        + XMVectorScale(XMVectorNegate(fwd), offPos.z);

    XMStoreFloat3(&outLocal.pos, camV);

    _vec lookDir = XMVector3Normalize(XMLoadFloat3(&offLook));
    _vec worldUp = Utility::Up();
    _vec q = Utility::BuildLookRot(lookDir, worldUp);
    XMStoreFloat4(&outLocal.rot, q);

    Lens lens{};
    lens.fovY = XMConvertToRadians(fovDeg);
    lens.nearZ = 0.1f;
    lens.farZ = 5000.f;
    outLocal.lens = lens;
}

float CamRegistry::EaseInOut(float x) const
{
    if (x <= 0.f) return 0.f;
    if (x >= 1.f) return 1.f;

    return x * x * (3.f - 2.f * x);
}

void CamRegistry::CaptureKeyFromCamera(SeqCamPreset& preset, size_t keyIdx)
{
    const _float4x4* camWorld = tfSys->GetWorld(debugCamTf);
    if (!camWorld) return;

    _float3 camPos{ camWorld->_41, camWorld->_42, camWorld->_43 };

    _float3 leaderPos{}, focusPos{};
    _vec    right{}, up{}, fwd{};
    if (!ComputeFocusPos(leaderPos, focusPos, right, up, fwd))
        return;

    _vec camV = XMLoadFloat3(&camPos);
    _vec leaderV = XMLoadFloat3(&leaderPos);
    _vec delta = XMVectorSubtract(camV, leaderV);

    float x = XMVectorGetX(XMVector3Dot(delta, right));
    float y = XMVectorGetX(XMVector3Dot(delta, up));
    float z = XMVectorGetX(XMVector3Dot(delta, XMVectorNegate(fwd)));

    if (keyIdx >= preset.keys.size())
        preset.keys.resize(keyIdx + 1);

    CamKey& key = preset.keys[keyIdx];
    key.pos = _float3{ x, y, z };

    _vec forwardV = XMVector3Normalize(
        XMVectorSet(camWorld->_31, camWorld->_32, camWorld->_33, 0.f)
    );
    XMStoreFloat3(&key.look, forwardV);

    Handle camHandle = debugCamCam.IsValid() ? debugCamCam : camSys->GetMainCamHandle();
    if (camHandle.IsValid())
    {
        auto* cam = camSys->Get(camHandle);
        key.fovDeg = XMConvertToDegrees(cam->fovY);
    }
}

_float3 CamRegistry::EvalCatmullRom(const _float3& p0, const _float3& p1, const _float3& p2, const _float3& p3, float u) const
{
    if (u <= 0.f) u = 0.f;
    if (u >= 1.f) u = 1.f;

    float u2 = u * u;
    float u3 = u2 * u;

    _float3 r{};

    r.x = 0.5f * ((2.f * p1.x) +  (-p0.x + p2.x) * u + (2.f * p0.x - 5.f * p1.x + 4.f * p2.x - p3.x) * u2 + (-p0.x + 3.f * p1.x - 3.f * p2.x + p3.x) * u3);
    r.y = 0.5f * ((2.f * p1.y) +  (-p0.y + p2.y) * u + (2.f * p0.y - 5.f * p1.y + 4.f * p2.y - p3.y) * u2 + (-p0.y + 3.f * p1.y - 3.f * p2.y + p3.y) * u3);
    r.z = 0.5f * ((2.f * p1.z) +  (-p0.z + p2.z) * u + (2.f * p0.z - 5.f * p1.z + 4.f * p2.z - p3.z) * u2 + (-p0.z + 3.f * p1.z - 3.f * p2.z + p3.z) * u3);

    return r;
}

TrackID CamRegistry::Spawn(const TrackSpawnRequest& req, bool trackRecent)
{
    TrackID id = director->Spawn(req);
    if (!id.IsValid()) return {};

    if (req.type == CamTrackType::Follow)
        baseFollowId = id;

    if (req.type == CamTrackType::Sequence)
    {
        auto it = introClips.find(req.seqDesc.clipId);
        if (it != introClips.end())
            director->SetSeqClips(id, it->second);
    }

    if (trackRecent)
        recent.push_back(id);

    return id;
}

void CamRegistry::SetDebugCam(Handle cam, Handle tf)
{
    debugCamCam = cam;
    debugCamTf = tf;
}

void CamRegistry::RegisterDefaults()
{
    TrackSpawnRequest follow{};
    follow.type                    = CamTrackType::Follow;
    follow.priority                = CamPriority::Default;
    follow.layer                   = CamLayer::Base;
    follow.anchor.space            = AnchorSpace::Target;
    follow.anchor.binding          = TargetBinding::Leader;
    follow.anchor.offset           = { 0.f, 0.f, 0.f };
    follow.followDesc.orbitRadius  = 265.f;
    follow.followDesc.orbitHeight  = 150.f;
    follow.followDesc.yawOffsetDeg = 30.f;
    follow.followDesc.aimOffsetY   = 100.f;

    presets.emplace((int)BattleCamKey::Default_Follow, follow);
}

bool CamRegistry::ComputeFocusPos(_float3& outLeaderPos, _float3& outFocusPos, _vec& outRight, _vec& outUp, _vec& outForward) const
{
    EntityID leader = timelineSys->GetLeader();
    return ComputeFocusPosFor(leader, outLeaderPos, outFocusPos, outRight, outUp, outForward);
}

ClipId CamRegistry::RegisterCamClip(const filesystem::path& path)
{
    ClipId       loadedClip{};
    SeqCamPreset loadedPreset{};
    float        baseFovDeg = 60.f;
    FollowTrackDesc follow{};

    bool ok = camSerializer->Load(loadedClip, loadedPreset, baseFovDeg, follow, path);
    assert(ok);
    assert(!loadedPreset.keys.empty());

    seqPresets[loadedClip] = loadedPreset;

    if (!loadedPreset.clips.empty())
        RegisterSeqClips(loadedClip, loadedPreset.clips);

    const CamMeta& meta = seqPresets[loadedClip].meta;

    if (meta.ownerType == CamOwnerType::Character)
    {
        if (meta.role == CamRole::BasicAttack)
            basicAttackIdx[meta.characterId] = loadedClip;

        if (meta.role == CamRole::Reward)
            rewardIdx[meta.characterId] = loadedClip;

        if (meta.specialTag != SpecialAnimTag::None)
        {
            ActionCamSpec spec{};
            spec.clipId = loadedClip;
            spec.anchor = ActionCamAnchor::Attacker;
            spec.priority = CamPriority::Cinematic;
            spec.layer = CamLayer::Action;
            spec.lockUntilEnd = (meta.role != CamRole::BasicAttack);
            spec.fovOverride = 0.f;

            actionCamReg->RegisterSkillCam(meta.characterId, meta.specialTag, spec, loadedPreset.clips);
        }
    }

    samplers[loadedClip] =
        [this](ClipId clip, double tLocal, const SequenceTrackDesc& desc, CamPose& outLocal) -> bool
        {
            auto it = seqPresets.find(clip);
            assert(it != seqPresets.end());

            float t = static_cast<float>(tLocal);
            EvalSeqEvent(clip, it->second, t, outLocal);
            return true;
        };

    if (meta.role == CamRole::BattleIntro)
    {
        TrackSpawnRequest intro{};
        intro.type = CamTrackType::Sequence;
        intro.priority = CamPriority::Cinematic;
        intro.layer = CamLayer::Action;

        intro.anchor.space = AnchorSpace::Target;
        intro.anchor.binding = TargetBinding::Leader;
        intro.anchor.offset = _float3{ 0.f, 0.f, 0.f };

        intro.seqDesc.clipId = loadedClip;
        intro.seqDesc.lockUntilEnd = false; // ★ 인트로는 알아서 끝나면 죽게
        intro.seqDesc.loop = false;
        intro.seqDesc.timeScale = 1.f;

        presets.emplace((int)BattleCamKey::Intro_Action, intro);
    }

    return loadedClip;
}

ClipId CamRegistry::FindBasicAttackCam(CharacterID characterId) const
{
    auto it = basicAttackIdx.find(characterId);
    if (it == basicAttackIdx.end()) return {};
    return it->second;
}

void CamRegistry::StopActionTrack(TrackID id)
{
    if (!id.IsValid())
        return;

    CamPose now = director->GetOutput();
  
    if (baseFollowId.IsValid())
        director->SnapTrackToPose(baseFollowId, now);

    TrackKillRequest kill{};
    kill.id = id;
    director->Kill(kill);

    recent.erase(  remove_if(recent.begin(), recent.end(), [id](TrackID t) { return t.idx == id.idx && t.gen == id.gen; }), recent.end());
}

void CamRegistry::SmoothBackToFollow()
{
    CamPose now = director->GetOutput();

    if (baseFollowId.IsValid())
    {
        director->SnapTrackToPose(baseFollowId, now);
        director->SetGoal(baseFollowId, now);
    }

    if (actionTrackId.IsValid())
    {
        TrackKillRequest kill{};
        kill.id = actionTrackId;
        kill.immediate = false;
        director->Kill(kill);

        actionTrackId = {};
    }
}

float CamRegistry::ComputeCamDuration(const SeqCamPreset& preset)
{
    float dur = preset.duration;

    if (!preset.keys.empty())
        dur = max(dur, preset.keys.back().t);

    for (const CamShakeEvent& s : preset.shakes)
    {
        float end = s.t + s.width * 0.5f;
        dur = max(dur, end);
    }
    return dur;
}

ClipId CamRegistry::FindRewardCam(CharacterID characterId) const
{
    auto it = rewardIdx.find(characterId);
    if (it == rewardIdx.end()) return {};
    return it->second;
}

TrackID CamRegistry::PlayRewardCam(CharacterID characterId, EntityID entity)
{
    ClipId clip = FindRewardCam(characterId);
    if (clip == 0)
        return {};

    seqBasisEntity[clip] = entity;

    TrackSpawnRequest req{};
    req.type = CamTrackType::Sequence;
    req.priority = CamPriority::Cinematic;
    req.layer = CamLayer::Action;

    req.anchor.space = AnchorSpace::Target;
    req.anchor.binding = TargetBinding::CustomEntity;
    req.anchor.entity = entity;

    req.seqDesc.clipId = clip;
    req.seqDesc.lockUntilEnd = true;
    req.seqDesc.loop = false;
    req.seqDesc.timeScale = 1.f;

    return Spawn(req, true);
}

bool CamRegistry::ComputeFocusPosFor(EntityID leader, _float3& outLeaderPos, _float3& outFocusPos, _vec& outRight, _vec& outUp, _vec& outForward) const
{
    if (leader == 0u)
        return false;

    Handle        leaderTf{};
    TransformData* leaderData = tfSys->GetByOwner(leader, &leaderTf);
    if (!leaderData)
        return false;

    const _float4x4* leaderWorld = tfSys->GetWorld(leaderTf);
    if (!leaderWorld)
        return false;

    _float3 leaderPos{ leaderWorld->_41, leaderWorld->_42, leaderWorld->_43 };

    EntityID target = targetSys->Get(leader);
    _float3 targetPos = leaderPos;

    if (target != 0u)
    {
        Handle         targetTf{};
        TransformData* targetData = tfSys->GetByOwner(target, &targetTf);
        if (targetData)
        {
            const _float4x4* targetWorld = tfSys->GetWorld(targetTf);
            if (targetWorld)
                targetPos = _float3{ targetWorld->_41, targetWorld->_42, targetWorld->_43 };
        }
    }

    const float alpha = 0.6f;
    _float3 focusPos{
        leaderPos.x + (targetPos.x - leaderPos.x) * alpha,
        leaderPos.y + (targetPos.y - leaderPos.y) * alpha,
        leaderPos.z + (targetPos.z - leaderPos.z) * alpha
    };

    float dx = targetPos.x - leaderPos.x;
    float dy = targetPos.y - leaderPos.y;
    float dz = targetPos.z - leaderPos.z;
    float lenSq = dx * dx + dy * dy + dz * dz;

    _vec fwd{};

    if (lenSq < 1e-6f)
    {
        _float2 forwardXZ = tfSys->GetForwardXZ(leaderTf);
        fwd = XMVector3Normalize(XMVectorSet(forwardXZ.x, 0.f, forwardXZ.y, 0.f));
    }
    else
        fwd = XMVector3Normalize(XMVectorSet(dx, dy, dz, 0.f));

    _vec up = Utility::Up();
    _vec right = XMVector3Normalize(XMVector3Cross(up, fwd));
    up = XMVector3Normalize(XMVector3Cross(fwd, right));

    outLeaderPos = leaderPos;
    outFocusPos  = focusPos;
    outRight     = right;
    outUp        = up;
    outForward   = fwd;
    return true;
}

CamFadeProfile CamRegistry::GetFadeProfile(CamRole role) const
{
    CamFadeProfile cfg{};

    switch (role)
    {
    case CamRole::Reward:
        cfg.useFade = true;
        cfg.inDur   = 0.35f;
        cfg.outDur  = 0.35f;
        cfg.mode    = FadeMode::White;
        break;

    case CamRole::CutScene:
        cfg.useFade = true;
        cfg.inDur   = 0.15f;
        cfg.outDur  = 0.15f;
        cfg.mode    = FadeMode::White;
        break;

    default:
        cfg.useFade = false;  
        break;
    }
    return cfg;
}

void CamRegistry::RenderGui(EntityID id)
{
#ifdef USE_IMGUI

    ImGui::Separator();
    static bool useDebugCamAsBattleCam = false;
    bool prev = useDebugCamAsBattleCam;
    ImGui::Checkbox("Use DebugCam as Battle Camera", &useDebugCamAsBattleCam);

    if (useDebugCamAsBattleCam != prev)
    {
        if (useDebugCamAsBattleCam)
        {
            prevMainCam = camSys->GetMainCamHandle();
            if (debugCamCam.IsValid())
            {
                camSys->SetMainCam(debugCamCam, true);
                debugCamOverride = true;
            }
            else
            {
                ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "DebugCam not set!");
                useDebugCamAsBattleCam = false;
            }
        }
        else
        {
            if (debugCamOverride && prevMainCam.IsValid())
                camSys->SetMainCam(prevMainCam, true);
            debugCamOverride = false;
        }
    }

    // ----------------------------------------------------------------
    // Base Lens
    // ----------------------------------------------------------------
    {
        ImGui::Separator();
        ImGui::Text("Base Lens");

        Lens lens = director->GetFixedLens();
        float fovDeg = XMConvertToDegrees(lens.fovY);

        if (ImGui::DragFloat("Base FOV (deg)", &fovDeg, 0.1f, 10.f, 120.f))
        {
            lens.fovY = XMConvertToRadians(fovDeg);
            director->SetFixedLens(lens);
        }
    }

    // ----------------------------------------------------------------
    // Follow Camera (Default_Follow)
    // ----------------------------------------------------------------
    {
        ImGui::Separator();
        ImGui::Text("Follow Camera (Default_Follow)");

        FollowTrackDesc* live = nullptr;
        if (baseFollowId.IsValid())
            live = &director->GetFollowDesc(baseFollowId);

        TrackSpawnRequest* followPreset = nullptr;
        auto it = presets.find((int)BattleCamKey::Default_Follow);
        if (it != presets.end())
            followPreset = &it->second;

        if (live)
        {
            ImGui::DragFloat("Orbit Radius", &live->orbitRadius, 1.f, 50.f, 2000.f);
            ImGui::DragFloat("Orbit Height", &live->orbitHeight, 1.f, 0.f, 1000.f);
            ImGui::DragFloat("Yaw Offset Deg", &live->yawOffsetDeg, 1.f, -180.f, 180.f);
            ImGui::DragFloat("Aim Offset Y", &live->aimOffsetY, 1.f, -200.f, 400.f);

            if (followPreset)
                followPreset->followDesc = *live;
        }
        else
            ImGui::TextDisabled("Follow track not alive.");
    }

    // ----------------------------------------------------------------
    // Sequence Preset 선택
    // ----------------------------------------------------------------
    ImGui::Separator();

    static ClipId curClip = 2101;
    ImGui::InputInt("ClipId", (int*)&curClip);

    SeqCamPreset* preset = nullptr;
    {
        auto it = seqPresets.find(curClip);
        if (it != seqPresets.end())
            preset = &it->second;
    }

    if (!preset)
    {
        ImGui::Text("No preset for clipId = %u", curClip);
        if (ImGui::Button("Create New Preset"))
        {
            SeqCamPreset p{};
            p.duration = 3.f;
            seqPresets[curClip] = p;
        }
        return;
    }

    static TrackID previewTrack{};

    EntityID leader = timelineSys ? timelineSys->GetLeader() : 0u;
    if (leader == 0u)
    {
        ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "No leader: cannot preview sequence.");
    }
    else if (ImGui::Button("Play Preview Sequence") && director)
    {
        // 이전 프리뷰 트랙 정리
        if (previewTrack.IsValid())
        {
            TrackKillRequest kill{};
            kill.id = previewTrack;
            director->Kill(kill);
            previewTrack = {};
        }

        samplers[curClip] =
            [this](ClipId clip, double tLocal, const SequenceTrackDesc& desc, CamPose& outLocal) -> bool
            {
                auto it = seqPresets.find(clip);
                if (it == seqPresets.end()) return false;

                float t = static_cast<float>(tLocal);
                EvalSeqEvent(clip, it->second, t, outLocal);
                return true;
            };

        TrackSpawnRequest req{};
        req.type = CamTrackType::Sequence;
        req.priority = CamPriority::Cinematic;
        req.layer = CamLayer::Action;


        req.anchor.space = AnchorSpace::Target;
        req.anchor.binding = TargetBinding::Leader;

        req.seqDesc.clipId = curClip;
        req.seqDesc.lockUntilEnd = false;
        req.seqDesc.loop = false;
        req.seqDesc.timeScale = 1.f;

        previewTrack = Spawn(req, false);
    }

    ImGui::SameLine();

    if (ImGui::Button("Stop Preview") && director)
    {
        if (previewTrack.IsValid())
        {
            TrackKillRequest kill{};
            kill.id = previewTrack;
            director->Kill(kill);
            previewTrack = {};
        }
    }

    // ----------------------------------------------------------------
    // Cam Meta
    // ----------------------------------------------------------------
    ImGui::Separator();
    ImGui::Text("Cam Meta");

    CamMeta& meta = preset->meta;

    // OwnerType
    int ownerTypeInt = (meta.ownerType == CamOwnerType::Default) ? 0 : 1;
    ImGui::Combo("OwnerType", &ownerTypeInt, "Default\0Character\0");
    meta.ownerType = (ownerTypeInt == 0) ? CamOwnerType::Default : CamOwnerType::Character;

    // Role
    int roleInt = 0;
    switch (meta.role)
    {
    case CamRole::BasicAttack:  roleInt = 0; break;
    case CamRole::SkillSlot:    roleInt = 1; break;
    case CamRole::Ultimate:     roleInt = 2; break;
    case CamRole::BattleIntro:  roleInt = 3; break;
    case CamRole::Reward:       roleInt = 4; break;
    case CamRole::CutScene:     roleInt = 5; break;
    }

    ImGui::Combo("Role", &roleInt,
        "BasicAttack\0"
        "SkillSlot\0"
        "Ultimate\0"
        "BattleIntro\0"
        "Reward\0"
        "CutScene\0");

    switch (roleInt)
    {
    case 0: meta.role = CamRole::BasicAttack;  break;
    case 1: meta.role = CamRole::SkillSlot;    break;
    case 2: meta.role = CamRole::Ultimate;     break;
    case 3: meta.role = CamRole::BattleIntro;  break;
    case 4: meta.role = CamRole::Reward;       break;
    case 5: meta.role = CamRole::CutScene;     break;
    }

    // CharacterID
    {
        static const CharacterID characterEnums[] =
        {
            CharacterID::Unknown,
            CharacterID::Ryza,
            CharacterID::Klaudia,
            CharacterID::Patricia,
        };

        static const char* characterLabels[] =
        {
            "Unknown",
            "Ryza",
            "Klaudia",
            "Patricia",
        };

        const int characterCount = (int)(sizeof(characterEnums) / sizeof(characterEnums[0]));

        int curIndex = 0;
        for (int i = 0; i < characterCount; ++i)
        {
            if (characterEnums[i] == meta.characterId)
            {
                curIndex = i;
                break;
            }
        }

        if (ImGui::Combo("Character", &curIndex, characterLabels, characterCount))
            meta.characterId = characterEnums[curIndex];
    }

    // SpecialAnimTag
    {
        static const SpecialAnimTag tagEnums[] =
        {
            SpecialAnimTag::None,
            SpecialAnimTag::BasicAttack,
            SpecialAnimTag::SkillA_1,
            SpecialAnimTag::SkillA_2,
            SpecialAnimTag::SkillA_3,
        };

        static const char* tagLabels[] =
        {
            "None",
            "BasicAttack",
            "SkillA_1",
            "SkillA_2",
            "SkillA_3",
        };

        const int tagCount = (int)(sizeof(tagEnums) / sizeof(tagEnums[0]));

        int curIndex = 0;
        for (int i = 0; i < tagCount; ++i)
        {
            if (tagEnums[i] == meta.specialTag)
            {
                curIndex = i;
                break;
            }
        }

        if (ImGui::Combo("SpecialAnimTag", &curIndex, tagLabels, tagCount))
            meta.specialTag = tagEnums[curIndex];
    }

    // ----------------------------------------------------------------
    // Timeline + Preview
    // ----------------------------------------------------------------
    ImGui::DragFloat("Duration", &preset->duration, 0.01f, 0.f, 30.f);

    static float previewT = 0.f;
    float dur = preset->duration;
    if (dur <= 0.f) dur = 0.001f;

    ImGui::Text("Timeline");
    ImVec2 barSize(ImGui::GetContentRegionAvail().x, 18.f);
    ImGui::InvisibleButton("SeqTimeline", barSize);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p0 = ImGui::GetItemRectMin();
    ImVec2 p1 = ImGui::GetItemRectMax();
    float w = p1.x - p0.x;
    float h = p1.y - p0.y;

    dl->AddRectFilled(p0, p1, IM_COL32(40, 40, 40, 255));
    dl->AddRect(p0, p1, IM_COL32(120, 120, 120, 255));

    for (int i = 0; i <= 4; ++i)
    {
        float u = i / 4.f;
        float x = p0.x + u * w;
        dl->AddLine(ImVec2(x, p0.y), ImVec2(x, p0.y + h), IM_COL32(70, 70, 70, 255));
    }

    for (const CamKey& k : preset->keys)
    {
        float u = k.t / dur;
        if (u < 0.f) u = 0.f;
        if (u > 1.f) u = 1.f;
        float x = p0.x + u * w;
        dl->AddLine(ImVec2(x, p0.y), ImVec2(x, p1.y), IM_COL32(80, 160, 255, 255));
    }

    for (const CamShakeEvent& s : preset->shakes)
    {
        float u = s.t / dur;
        if (u < 0.f) u = 0.f;
        if (u > 1.f) u = 1.f;
        float x = p0.x + u * w;
        dl->AddLine(ImVec2(x, p0.y), ImVec2(x, p1.y), IM_COL32(255, 160, 50, 255));
    }

    {
        ImGuiIO& io = ImGui::GetIO();
        bool hovered = ImGui::IsItemHovered();
        if (hovered && ImGui::IsMouseDown(0))
        {
            float u = (io.MousePos.x - p0.x) / w;
            if (u < 0.f) u = 0.f;
            if (u > 1.f) u = 1.f;
            previewT = u * dur;
        }

        float u = previewT / dur;
        if (u < 0.f) u = 0.f;
        if (u > 1.f) u = 1.f;
        float x = p0.x + u * w;
        dl->AddLine(ImVec2(x, p0.y), ImVec2(x, p1.y), IM_COL32(255, 60, 60, 255), 2.f);
    }

    ImGui::SliderFloat("Preview t", &previewT, 0.f, preset->duration);

    // ----------------------------------------------------------------
    // Cam Keys
    // ----------------------------------------------------------------
    if (ImGui::CollapsingHeader("Cam Keys", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::PushID("Keys");

        for (size_t i = 0; i < preset->keys.size(); ++i)
        {
            ImGui::PushID((int)i);

            ImGui::DragFloat("t", &preset->keys[i].t, 0.01f, 0.f, preset->duration);

            ImGui::Text("Pos  : (%.1f, %.1f, %.1f)",
                preset->keys[i].pos.x,
                preset->keys[i].pos.y,
                preset->keys[i].pos.z);

            ImGui::Text("Look : (%.1f, %.1f, %.1f)",
                preset->keys[i].look.x,
                preset->keys[i].look.y,
                preset->keys[i].look.z);

            ImGui::DragFloat("FOV Deg", &preset->keys[i].fovDeg, 0.1f, 10.f, 120.f);

            if (ImGui::Button("Capture From Camera"))
                CaptureKeyFromCamera(*preset, i);

            ImGui::SameLine();
            if (ImGui::Button("Remove"))
            {
                preset->keys.erase(preset->keys.begin() + i);
                ImGui::PopID();
                break;
            }

            ImGui::Separator();
            ImGui::PopID();
        }

        if (ImGui::Button("Add Key"))
        {
            CamKey k{};
            k.t = previewT;
            k.pos = _float3{};
            k.look = _float3{};
            k.fovDeg = 60.f;

            preset->keys.push_back(k);

            size_t newIdx = preset->keys.size() - 1;
            CaptureKeyFromCamera(*preset, newIdx);
        }

        ImGui::PopID();
    }

    // ----------------------------------------------------------------
    // Shakes
    // ----------------------------------------------------------------
    if (ImGui::CollapsingHeader("Shakes", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::PushID("Shakes");

        for (size_t i = 0; i < preset->shakes.size(); ++i)
        {
            ImGui::PushID((int)i);

            ImGui::DragFloat("t", &preset->shakes[i].t, 0.01f, 0.f, preset->duration);
            ImGui::DragFloat("width", &preset->shakes[i].width, 0.01f, 0.f, 1.f);
            ImGui::DragFloat("amp", &preset->shakes[i].amp, 0.1f, 0.f, 50.f);
            ImGui::DragFloat("yScale", &preset->shakes[i].yScale, 0.01f, 0.f, 2.f);

            if (ImGui::Button("Remove"))
            {
                preset->shakes.erase(preset->shakes.begin() + i);
                ImGui::PopID();
                break;
            }

            ImGui::Separator();
            ImGui::PopID();
        }

        if (ImGui::Button("Add Shake"))
        {
            CamShakeEvent s{};
            s.t = preset->duration * 0.5f;
            s.width = 0.1f;
            s.amp = 5.f;
            s.yScale = 0.5f;
            preset->shakes.push_back(s);
        }

        ImGui::PopID();
    }

    // ----------------------------------------------------------------
    // Save / Load
    // ----------------------------------------------------------------
    ImGui::Separator();

    static char camFile[260] = "patricia_skillA1.cam";
    ImGui::InputText("Cam File", camFile, sizeof(camFile));

    const wstring camFilter =
        L"Camera Preset (*.cam)\0*.cam\0"
        L"All Files (*.*)\0*.*\0";

    // Save
    if (ImGui::Button("Save Cam") && camSerializer)
    {
        filesystem::path initDir = L"Data/Cam";
        wstring defaultName = Utility::ToWString(string(camFile));

        auto pathOpt = Utility::SaveFileDialog(
            camFilter,
            defaultName,
            L".cam",
            initDir
        );

        if (pathOpt)
        {
            filesystem::path path = *pathOpt;

            float baseFovDeg = 60.f;
            FollowTrackDesc follow{};

            if (director)
            {
                Lens lens = director->GetFixedLens();
                baseFovDeg = XMConvertToDegrees(lens.fovY);
            }

            if (baseFollowId.IsValid())
                follow = director->GetFollowDesc(baseFollowId);
            else
            {
                auto itFollow = presets.find((int)BattleCamKey::Default_Follow);
                if (itFollow != presets.end())
                    follow = itFollow->second.followDesc;
            }

            bool ok = camSerializer->Save(curClip, *preset, baseFovDeg, follow, path);
            if (ok)
            {
                string utf8Name = Utility::ToString(path.filename().wstring());
                strncpy_s(camFile, utf8Name.c_str(), sizeof(camFile) - 1);
            }
        }
    }

    ImGui::SameLine();

    // Load
    if (ImGui::Button("Load Cam") && camSerializer)
    {
        filesystem::path initDir = L"../bin/Resources/Camera/";
        auto pathOpt = Utility::OpenFileDialog( camFilter, L".cam", initDir );
        if (pathOpt)
        {
            filesystem::path path = *pathOpt;

            ClipId loadedClip = RegisterCamClip(path);
            curClip = loadedClip;
            previewT = 0.f;

            string utf8Name = Utility::ToString(path.filename().wstring());
            strncpy_s(camFile, utf8Name.c_str(), sizeof(camFile) - 1);
        }
    }

#endif
}
