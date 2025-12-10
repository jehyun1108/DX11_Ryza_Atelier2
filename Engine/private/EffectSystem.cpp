#include "Enginepch.h"
#include "EffectSystem.h"

// =======================================================================
void EffectSystem::OnBoot()
{
	particleSys  = &registry.Get<ParticleSystem>();
	tfSys        = &registry.Get<TransformSystem>();
    entityMgr    = &registry.Get<EntityMgr>();
    assets       = &registry.Get<AssetSystem>();
    trailSys     = &registry.Get<TrailSystem>();
    collisionSys = &registry.Get<CollisionSystem>();
    effectOwner  = 0u;

    TextureMeta meta;
    meta.colorSpace = TextureColorSpace::Linear;
    meta.fullPath = L"../bin/Resources/Particles/particle_default.dds";
    assets->RegisterTexture(L"particle_default", meta);

    meta.fullPath = L"../bin/Resources/Particles/trail_default.dds";
    assets->RegisterTexture(L"trail_default", meta);
}

void EffectSystem::Tick(float dt)
{
    for (_uint idx = 0; idx < instances.size(); ++idx)
    {
        EffectInstance& inst = instances[idx];
        if (inst.handle == 0u) continue;
        if (!inst.archetype)   continue;

        UpdateInstance(inst, idx, dt);
    }
}

EffectHandle EffectSystem::PlayAt(const wstring& key, const _float3& worldPos)
{
    const EffectArchetype* arche = FindArchetype(key);
    EffectInstance& inst = NewInstance(arche);
    inst.attachType  = EffectAttachType::None;
    inst.worldPos    = worldPos;
    inst.localOffset = _float3(0.f, 0.f, 0.f);
    inst.attachOwner = 0u;
    return inst.handle;
}

EffectHandle EffectSystem::PlayAttached(const wstring& key, EntityID attachOwner, const _float3& localOffset)
{
    Handle tfHandle{};
    TransformData* tf    = tfSys->GetByOwner(attachOwner, &tfHandle);
    auto* archetype      = FindArchetype(key);
    EffectInstance& inst = NewInstance(archetype);
    inst.attachType      = EffectAttachType::Transform;
    inst.owner           = effectOwner;
    inst.attachOwner     = attachOwner;
    inst.followTf        = tfHandle;
    inst.localOffset     = localOffset;
    inst.worldPos        = tf->pos;   
    return inst.handle;
}

EffectHandle EffectSystem::PlayTrail(const wstring& key, EntityID attachOwner, float duration, float startDelay)
{
    EffectHandle h = PlayAttached(key, attachOwner);

    if (duration > 0.f || startDelay > 0.f)
    {
        EffectInstance* inst = GetInstance(h);

        if (duration > 0.f)
            inst->durationOverride = duration;

        if (startDelay > 0.f)
            inst->startDelay = startDelay;
    }
    return h;
}

void EffectSystem::Stop(EffectHandle handle)
{
    particleSys->KillByOwner(handle);

    EffectInstance* inst = GetInstance(handle);
    const EffectArchetype* arche = inst->archetype;

    for (auto& rt : inst->emitters)
    {
        if (rt.active && rt.spawner.IsValid())
        {
            particleSys->DestroySpawner(rt.spawner);
            rt.spawner = {};
            rt.active = false;
        }

        if (rt.trail.IsValid())
        {
            trailSys->Destroy(rt.trail);
            rt.trail = {};
        }
    }

    inst->finished = true;
    inst->archetype = nullptr;
    inst->handle = 0u;

    _uint idx = handle - 1u;
    freeList.push_back(idx);
}

bool EffectSystem::IsAlive(EffectHandle handle) const
{
    const EffectInstance* inst = GetInstance(handle);
    if (!inst) return false;
    return !inst->finished;
}

EffectInstance* EffectSystem::GetInstance(EffectHandle handle)
{
    if (handle == 0u) return nullptr;
    _uint idx = handle - 1u;
    if (idx >= instances.size()) return nullptr;
    EffectInstance& inst = instances[idx];
    if (inst.handle != handle) return nullptr;
    return &inst;
}                                                                                                                                                     

const EffectInstance* EffectSystem::GetInstance(EffectHandle handle) const
{
    if (handle == 0u) return nullptr;
    _uint idx = handle - 1u;
    if (idx >= instances.size()) return nullptr;
    const EffectInstance& inst = instances[idx];
    if (inst.handle != handle) return nullptr;
    return &inst;
}

EffectInstance& EffectSystem::NewInstance(const EffectArchetype* archetype)
{
    if (effectOwner == 0u)
        effectOwner = entityMgr->Create();

    _uint idx;
    if (!freeList.empty())
    {
        idx = freeList.back();
        freeList.pop_back();
    }
    else
    {
        idx = static_cast<_uint>(instances.size());
        instances.emplace_back();
    }

    EffectInstance& inst  = instances[idx];
    inst                  = {};
    inst.handle           = idx + 1u;
    inst.owner            = effectOwner;
    inst.archetype        = archetype;
    inst.elapsed          = 0.f;
    inst.finished         = false;
    inst.durationOverride = 0.f;
    inst.startDelay       = 0.f;
    inst.attachType       = EffectAttachType::None;
    inst.followTf         = {};
    inst.attachOwner      = 0u;
    inst.worldPos         = _float3(0.f, 0.f, 0.f);
    inst.localOffset      = _float3(0.f, 0.f, 0.f);

    inst.emitters.clear();
    inst.emitters.reserve(archetype->emitters.size());

    for (int i = 0; i < static_cast<int>(archetype->emitters.size()); ++i)
    {
        EffectEmitterRuntime rt{};
        rt.emitterIdx = i;
        inst.emitters.push_back(rt);
    }

    inst.nextEventIdx = 0;
    return inst;
}

void EffectSystem::RegisterArchetype(EffectArchetype& def)
{
    auto it = archetypes.find(def.key);
    if (it == archetypes.end())
    {
        auto ptr = make_shared<EffectArchetype>(def);
        archetypes[def.key] = move(ptr);
    }
    else
        *(it->second) = def; 
}

const EffectArchetype* EffectSystem::FindArchetype(const wstring& key) const
{
    auto it = archetypes.find(key);
    assert(it != archetypes.end());
    const shared_ptr<EffectArchetype>& ptr = it->second;
    assert(ptr);
    return ptr.get();
}

void EffectSystem::UpdateInstance(EffectInstance& inst, _uint idx, float dt)
{
    const EffectArchetype* arche = inst.archetype;
    if (inst.startDelay > 0.f)
    {
        if (inst.startDelay > dt)
        {
            inst.startDelay -= dt;
            return;
        }

        dt -= inst.startDelay;
        inst.startDelay = 0.f;
    }

    inst.elapsed += dt;

    float totalDur = arche->duration;
    if (inst.durationOverride > 0.f)
        totalDur = inst.durationOverride;

    if (totalDur > 0.f && inst.elapsed >= totalDur)
    {
        FinishInstance(inst, idx);
        return;
    }

    _float3 trailDir{ 0.f, 1.f, 0.f };
    UpdateAttachmentAndTrailDir(inst, trailDir);

    const float t = inst.elapsed;

    for (auto& rt : inst.emitters)
    {
        const EffectEmitterDesc& desc = arche->emitters[rt.emitterIdx];
        UpdateEmitter(inst, desc, rt, trailDir, t, dt);
    }
}

void EffectSystem::UpdateAttachmentAndTrailDir(EffectInstance& inst, _float3& outTrailDir)
{
    outTrailDir = _float3{ 0.f, 1.f, 0.f };

    if (inst.attachType != EffectAttachType::Transform) return;

    TransformData* tf = tfSys->Get(inst.followTf);

    inst.worldPos = tf->pos;

    _float3 up{
        tf->world._21,
        tf->world._22,
        tf->world._23
    };
    _vec u = XMLoadFloat3(&up);
    u = XMVector3Normalize(u);
    XMStoreFloat3(&outTrailDir, u);
}

void EffectSystem::UpdateEmitter(EffectInstance& inst, const EffectEmitterDesc& desc, EffectEmitterRuntime& rt, const _float3& trailDir, float t, float dt)
{
    const EffectArchetype* arche = inst.archetype;

    float totalDur = arche->duration;
    if (inst.durationOverride > 0.f)
        totalDur = inst.durationOverride;

    const float startTime = desc.delay;
    float       endTime = totalDur;

    if (desc.kind == EffectEmitterKind::Particle && desc.duration > 0.f)
        endTime = desc.delay + desc.duration;

    bool inRange = (t >= startTime) && (t <= endTime);

    if (debugTrailAlwaysOn && desc.kind == EffectEmitterKind::Trail)
        inRange = true;

    if (desc.kind == EffectEmitterKind::Particle)
        UpdateParticleEmitter(inst, desc, rt, inRange);
    else if (desc.kind == EffectEmitterKind::Trail)
        UpdateTrailEmitter(inst, desc, rt, trailDir, inRange, dt);
}

void EffectSystem::UpdateParticleEmitter(EffectInstance& inst, const EffectEmitterDesc& desc, EffectEmitterRuntime& rt, bool inRange)
{
    const ParticleSpawnData& spawnData = desc.particle;
    if (desc.burst)
    {
        if (inRange && !rt.burstFired)
        {
            _float3 spawnPos = CalcEmitterBasePosWorld(inst, desc);

            int count = desc.burstCount;
            if (count < 0) count = 0;
            if (count > 0)
                particleSys->SpawnBurst(spawnData, spawnPos, count, inst.handle);

            rt.burstFired = true;
        }
        return;
    }
    if (inRange)
    {
        if (!rt.active)
        {
            _float3 spawnPos = CalcEmitterBasePosWorld(inst, desc);
            rt.spawner = particleSys->CreateSpawner(inst.owner, &desc.particle, spawnPos);
            particleSys->SetSpawnerOwner(rt.spawner, inst.handle);
            rt.active  = true;

            if (inst.attachType == EffectAttachType::Transform)
            {
                TransformData* tf = tfSys->Get(inst.followTf);

                _float3 right   = { tf->world._11, tf->world._12, tf->world._13 };
                _float3 up      = { tf->world._21, tf->world._22, tf->world._23 };
                _float3 forward = { tf->world._31, tf->world._32,  tf->world._33};

                particleSys->SetSpawnerBasis(rt.spawner, right, up, forward, true);
            }
        }
        else
        {
            if (desc.spaceMode == EffectSpaceMode::Local)
            {
                _float3 spawnPos = CalcEmitterBasePosWorld(inst, desc);
                particleSys->SetSpawnerPos(rt.spawner, spawnPos);
            }
        }
    }
    else
    {
        if (rt.active)
        {
            if (rt.spawner.IsValid())
            {
                particleSys->DestroySpawner(rt.spawner);
                rt.spawner = {};
            }
            rt.active = false;
        }
    }
}

void EffectSystem::UpdateTrailEmitter( EffectInstance& inst, const EffectEmitterDesc& desc, EffectEmitterRuntime& rt, const _float3& trailDir, bool inRange, float dt)
{
    const TrailDesc& trailDesc = desc.trail;

    if (!inRange)
    {
        if (rt.active)
            rt.active = false;
        return;
    }

    _float3 tipPos{};
    float   arcT = 0.f;    
    bool    hasArcT = false;

    if (trailDesc.shapeMode == TrailShapeMode::ArcAnalytic)
    {
        float dur = (desc.duration > 0.f) ? desc.duration : trailDesc.lifeTime;
        if (dur <= 0.f) return;

        float localT = inst.elapsed - desc.delay;
        if (localT < 0.f && !rt.active) return;

        float s = Utility::Saturate(localT / dur);
        arcT = s;
        hasArcT = true;

        if (!rt.arcBasisInit)
        {
            TransformData* ownerTf = nullptr;
            if (inst.followTf.IsValid())
                ownerTf = tfSys->Get(inst.followTf);

            _float3 center{};
            if (trailDesc.arcUseownerCenter && ownerTf)
                center = ownerTf->pos;
            else
                center = inst.worldPos;

            center.x += trailDesc.arcCenterOffset.x;
            center.y += trailDesc.arcCenterOffset.y;
            center.z += trailDesc.arcCenterOffset.z;

            _float3 right{};
            _float3 forward{};

            if (trailDesc.arcPlane == TrailArcPlane::Horizontal)
            {
                right = _float3{ 1.f, 0.f, 0.f };
                forward = _float3{ 0.f, 0.f, 1.f };
            }
            else if (trailDesc.arcPlane == TrailArcPlane::Vertical)
            {
                right = _float3{ 1.f, 0.f, 0.f };
                forward = _float3{ 0.f, 1.f, 0.f };
            }
            else
            {
                right = _float3{ 1.f, 0.f, 0.f };
                forward = _float3{ 0.f, 0.f, 1.f };

                if (ownerTf)
                {
                    right = _float3{
                        ownerTf->world._11,
                        ownerTf->world._12,
                        ownerTf->world._13
                    };
                    forward = _float3{
                        ownerTf->world._31,
                        ownerTf->world._32,
                        ownerTf->world._33
                    };

                    _vec r = XMLoadFloat3(&right);
                    _vec f = XMLoadFloat3(&forward);
                    r = XMVector3Normalize(r);
                    f = XMVector3Normalize(f);
                    XMStoreFloat3(&right, r);
                    XMStoreFloat3(&forward, f);
                }
            }
            {
                float rx = XMConvertToRadians(trailDesc.arcRotDeg.x);
                float ry = XMConvertToRadians(trailDesc.arcRotDeg.y);
                float rz = XMConvertToRadians(trailDesc.arcRotDeg.z);

                _mat rot = XMMatrixRotationRollPitchYaw(rx, ry, rz);

                _vec vr = XMLoadFloat3(&right);
                _vec vf = XMLoadFloat3(&forward);

                vr = XMVector3TransformNormal(vr, rot);
                vf = XMVector3TransformNormal(vf, rot);

                XMStoreFloat3(&right, vr);
                XMStoreFloat3(&forward, vf);
            }

            rt.arcCenter = center;
            rt.arcRight = right;
            rt.arcForward = forward;
            rt.arcBasisInit = true;
        }

        float startRad = XMConvertToRadians(trailDesc.arcStartDeg);
        float endRad = XMConvertToRadians(trailDesc.arcEndDeg);
        float theta = startRad + (endRad - startRad) * arcT;

        float c = cosf(theta);
        float sAng = sinf(theta);
        float R = trailDesc.arcRadius;

        const _float3& center = rt.arcCenter;
        const _float3& right = rt.arcRight;
        const _float3& forward = rt.arcForward;

        _float3 offset{
            (right.x * c + forward.x * sAng) * R,
            (right.y * c + forward.y * sAng) * R,
            (right.z * c + forward.z * sAng) * R
        };

        tipPos.x = center.x + offset.x;
        tipPos.y = center.y + offset.y;
        tipPos.z = center.z + offset.z;
    }
    else
    {
        bool gotTip = false;

        if (desc.useColliderObbTip && inst.attachOwner != 0)
            gotTip = ComputeTrailTipFromCollider(inst, desc, tipPos);

        if (!gotTip)
            tipPos = CalcEmitterBasePosWorld(inst, desc);
    }
    if (!rt.active)
    {
        rt.trail = trailSys->Create(inst.owner, trailDesc);
        rt.active = true;
    }

    trailSys->AddSample(rt.trail, tipPos);
    if (trailDesc.sparkEnabled)
    {
        rt.sparkAccum += dt;

        float interval = trailDesc.sparkInterval;
        if (interval < 1e-4f) interval = 1e-4f;

        while (rt.sparkAccum >= interval)
        {
            rt.sparkAccum -= interval;

            int count = trailDesc.sparkBurstCount;
            if (count < 1) count = 1;

            float t = hasArcT ? arcT : Utility::Range(0.f, 1.f);
            ParticleSpawnData sparkData = trailDesc.spark;

            float lifeScale = 1.2f - 0.8f * t;   
            sparkData.lifeMin *= lifeScale;
            sparkData.lifeMax *= lifeScale;

            float sizeJitter = Utility::Range(0.7f, 1.3f);
            sparkData.startSize *= sizeJitter;
            sparkData.endSize *= sizeJitter;

            _float3 spawnPos = tipPos;
            float   r = Utility::Range(0.f, 5.f);
            float   ang = Utility::Range(0.f, XM_2PI);
            spawnPos.x += cosf(ang) * r;
            spawnPos.z += sinf(ang) * r;

            particleSys->SpawnBurst(sparkData, spawnPos, count, inst.handle);
        }
    }
}

void EffectSystem::FinishInstance(EffectInstance& inst, _uint idx)
{
    const EffectArchetype* arche = inst.archetype;
    for (auto& rt : inst.emitters)
    {
        const EffectEmitterDesc& desc = arche->emitters[rt.emitterIdx];

        if (desc.kind == EffectEmitterKind::Particle)
        {
            if (rt.active && rt.spawner.IsValid())
            {
                particleSys->DestroySpawner(rt.spawner);
                rt.spawner = {};
                rt.active = false;
            }
        }
        else if (desc.kind == EffectEmitterKind::Trail)
        {
            if (rt.trail.IsValid())
            {
                trailSys->Destroy(rt.trail);
                rt.trail = {};
            }
            rt.active = false;
        }
    }
    inst.finished  = true;
    inst.archetype = nullptr;
    inst.handle    = 0u;
    freeList.push_back(idx);
}

_float3 EffectSystem::CalcEmitterBasePosWorld(const EffectInstance& inst, const EffectEmitterDesc& desc)
{
    _float3 base = inst.worldPos;
    if (inst.attachType == EffectAttachType::Transform)
    {
        TransformData* tf = tfSys->Get(inst.followTf);
        assert(tf);

        _float3 right{
            tf->world._11,
            tf->world._12,
            tf->world._13
        };
        _float3 up{
            tf->world._21,
            tf->world._22,
            tf->world._23
        };
        _float3 forward{
            tf->world._31,
            tf->world._32,
            tf->world._33
        };

        _vec r = XMLoadFloat3(&right);
        _vec u = XMLoadFloat3(&up);
        _vec f = XMLoadFloat3(&forward);
        r = XMVector3Normalize(r);
        u = XMVector3Normalize(u);
        f = XMVector3Normalize(f);
        XMStoreFloat3(&right, r);
        XMStoreFloat3(&up, u);
        XMStoreFloat3(&forward, f);

        _float3 local{};
        local.x = inst.localOffset.x + desc.localOffset.x;
        local.y = inst.localOffset.y + desc.localOffset.y;
        local.z = inst.localOffset.z + desc.localOffset.z;

        _float3 rotated{};
        rotated.x = local.x * right.x + local.y * up.x + local.z * forward.x;
        rotated.y = local.x * right.y + local.y * up.y + local.z * forward.y;
        rotated.z = local.x * right.z + local.y * up.z + local.z * forward.z;

        base.x += rotated.x;
        base.y += rotated.y;
        base.z += rotated.z;
    }
    else
    {
        base.x += desc.localOffset.x;
        base.y += desc.localOffset.y;
        base.z += desc.localOffset.z;
    }
    return base;
}

bool EffectSystem::ComputeTrailTipFromCollider(const EffectInstance& inst, const EffectEmitterDesc& desc, _float3& outTip)
{
    if (inst.attachOwner == 0) return false;
    return collisionSys->GetObbTipPoint(inst.attachOwner, outTip);
}

void EffectSystem::ApplyEmitterPreset(EffectEmitterDesc& emitter, EmitterShapePreset preset)
{
    ParticleSpawnData& s = emitter.particle;

    emitter.kind = EffectEmitterKind::Particle;
    emitter.localOffset = _float3(0.f, 0.f, 0.f);
    emitter.spaceMode = EffectSpaceMode::Local;
    emitter.delay = 0.f;

    // “모양” 프리셋이니까, 시트/빌보드는 건드리지 말고
    // 기본은 BillBoard + sheet 끔
    s.visualMode = ParticleVisualMode::Billboard;
    s.sheet.enabled = false;

    switch (preset)
    {
    case EmitterShapePreset::Single:
    {
        emitter.burst = true;
        emitter.burstCount = 1;
        emitter.duration = 0.f; // 한 번만

        s.spawnRate = 0.f;   // burst only
        s.lifeMin = 0.6f;
        s.lifeMax = 0.8f;
        s.speedMin = 0.f;
        s.speedMax = 0.f;
        s.baseDir = _float3(0.f, 1.f, 0.f);
        s.spreadAng = 0.f;

        s.startSize = 80.f;
        s.endSize = 100.f;
        s.startColor = _float4(1.f, 1.f, 1.f, 1.f);
        s.endColor = _float4(1.f, 1.f, 1.f, 0.f);

        s.sizeCurve = EffectCurveType::EaseOut;
        s.alphaCurve = EffectCurveType::EaseOut;
        s.colorCurve = EffectCurveType::Linear;
        s.rateCurve = EffectCurveType::Linear;

        s.dirLocal = false;
        s.rotSpeedMin = 0.f;
        s.rotSpeedMax = 0.f;
        s.randomStartRot = false;
    }
    break;

    case EmitterShapePreset::RadialBurst:
    {
        emitter.burst = true;
        emitter.burstCount = 30;
        emitter.duration = 0.f;

        s.spawnRate = 0.f;   // burst only
        s.lifeMin = 0.5f;
        s.lifeMax = 0.8f;
        s.speedMin = 150.f;
        s.speedMax = 250.f;
        s.baseDir = _float3(0.f, 1.f, 0.f);
        s.spreadAng = XM_PI; // 주변 전체로

        s.startSize = 60.f;
        s.endSize = 40.f;
        s.startColor = _float4(1.f, 1.f, 1.f, 1.f);
        s.endColor = _float4(1.f, 1.f, 1.f, 0.f);

        s.sizeCurve = EffectCurveType::EaseOut;
        s.alphaCurve = EffectCurveType::EaseOut;
        s.colorCurve = EffectCurveType::Linear;
        s.rateCurve = EffectCurveType::Linear;

        s.dirLocal = false;
        s.rotSpeedMin = -10.f;
        s.rotSpeedMax = 10.f;
        s.randomStartRot = true;

        s.posRadiusMin = 5.f;
        s.posRadiusMax = 25.f;
        s.spreadAng = XM_PI;
    }
    break;
    }
}

void EffectSystem::RenderArchetypeGui(EffectArchetype& effect)
{
#ifdef USE_IMGUI
    RenderArchetypeHeader(effect);
    ImGui::SeparatorText("Emitters");
    RenderEmitterSection(effect);
    RenderEventSection(effect);
#endif
}

void EffectSystem::RenderArchetypeHeader(EffectArchetype& effect)
{
#ifdef USE_IMGUI
    string keyStr = Utility::ToString(effect.key);
    char   keyBuf[128]{};
    snprintf(keyBuf, sizeof(keyBuf), "%s", keyStr.c_str());

    if (ImGui::InputText("Key", keyBuf, IM_ARRAYSIZE(keyBuf)))
        effect.key = Utility::ToWString(string(keyBuf));

    ImGui::DragFloat("Duration", &effect.duration, 0.01f, 0.f, 9999.f);

    const char* layerNames[] = { "World", "UI", "PostProcess" };
    int layerIndex = static_cast<int>(effect.layer);
    if (layerIndex < 0 || layerIndex >= IM_ARRAYSIZE(layerNames))
        layerIndex = 0;
    if (ImGui::Combo("Layer", &layerIndex, layerNames, IM_ARRAYSIZE(layerNames)))
        effect.layer = static_cast<EffectRenderLayer>(layerIndex);

    ImGui::Text("Emitters: %d", static_cast<int>(effect.emitters.size()));
    ImGui::Text("Events:   %d", static_cast<int>(effect.events.size()));
#endif
}

void EffectSystem::RenderEmitterSection(EffectArchetype& effect)
{
#ifdef USE_IMGUI
    if (ImGui::Button("Add Emitter", ImVec2(-1, 0)))
    {
        EffectEmitterDesc em{};
        size_t idx = effect.emitters.size();

        em.name = L"Emitter" + to_wstring(idx);
        em.localOffset = _float3(0.f, 0.f, 0.f);
        em.spaceMode = EffectSpaceMode::Local;
        em.kind = EffectEmitterKind::Particle;

        em.particle.texKey = L"particle_default";

        ApplyEmitterPreset(em, EmitterShapePreset::Single);

        // 트레일 기본값 (Trail로 바꾸면 바로 쓸 수 있게)
        auto& td = em.trail;
        td.lifeTime = 0.3f;
        td.widthStart = 50.f;
        td.widthEnd = 0.f;
        td.colorStart = _float4(1.f, 1.f, 1.f, 1.f);
        td.colorEnd = _float4(1.f, 1.f, 1.f, 0.f);
        td.widthCurve = EffectCurveType::EaseOut;
        td.alphaCurve = EffectCurveType::EaseOut;
        td.minSegDist = 1.f;

        effect.emitters.push_back(em);
    }

    const char* curveNames[] = { "Linear", "EaseIn", "EaseOut", "EaseInOut", "Spike", "Bell", "Plateau" };

    int removeIndex    = -1;
    int duplicateIndex = -1;
    int moveUpIndex    = -1;
    int moveDownIndex  = -1;

    for (size_t i = 0; i < effect.emitters.size(); ++i)
        RenderSingleEmitter(effect, i, curveNames, IM_ARRAYSIZE(curveNames),
            removeIndex, duplicateIndex, moveUpIndex, moveDownIndex);

    ApplyEmitterCommands(effect, removeIndex, duplicateIndex, moveUpIndex, moveDownIndex);
#endif
}

void EffectSystem::RenderSingleEmitter( EffectArchetype& effect, size_t idx, const char** curveNames, int curveCount, int& removeIdx, int& duplicateIdx, int& moveUpIdx, int& moveDownIdx)
{
#ifdef USE_IMGUI
    auto& emitter = effect.emitters[idx];

    ImGui::PushID(static_cast<int>(idx));

    string emitterName = Utility::ToString(emitter.name);
    if (emitterName.empty())
        emitterName = "Emitter";

    char headerLabel[128]{};
    snprintf(headerLabel, sizeof(headerLabel), "Emitter[%d] : %s",
        static_cast<int>(idx), emitterName.c_str());

    if (ImGui::CollapsingHeader(headerLabel, ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Separator();
        ImGui::BeginGroup();
        ImGui::TextUnformatted("Controls");
        ImGui::SameLine();

        float btnSize = ImGui::GetFrameHeight();
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3.f, 2.f));

        bool canUp = (idx > 0);
        if (!canUp) ImGui::BeginDisabled();
        if (ImGui::Button("Up", ImVec2(btnSize, btnSize)))
            moveUpIdx = static_cast<int>(idx);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Move Up");
        if (!canUp) ImGui::EndDisabled();

        ImGui::SameLine();

        bool canDown = (idx + 1 < effect.emitters.size());
        if (!canDown) ImGui::BeginDisabled();
        if (ImGui::Button("Dn", ImVec2(btnSize, btnSize)))
            moveDownIdx = static_cast<int>(idx);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Move Down");
        if (!canDown) ImGui::EndDisabled();

        ImGui::SameLine();

        if (ImGui::Button("Dup", ImVec2(btnSize, btnSize)))
            duplicateIdx = static_cast<int>(idx);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Duplicate Emitter");

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.70f, 0.15f, 0.15f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.20f, 0.20f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.60f, 0.10f, 0.10f, 1.f));
        if (ImGui::Button("Del", ImVec2(btnSize, btnSize)))
            removeIdx = static_cast<int>(idx);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Delete Emitter");
        ImGui::PopStyleColor(3);

        ImGui::PopStyleVar();
        ImGui::EndGroup();
        // ======================

        ImGui::SeparatorText("Emitter Shape Preset");

        const char* shapeNames[] = { "Single", "Radial Burst" };
        int shapeIdx = 0;

        // 대충 현재 상태로부터 "표시용" 추정
        if (emitter.burst && emitter.burstCount == 1)
            shapeIdx = static_cast<int>(EmitterShapePreset::Single);
        else if (emitter.burst && emitter.burstCount > 1)
            shapeIdx = static_cast<int>(EmitterShapePreset::RadialBurst);

        if (ImGui::Combo("Shape", &shapeIdx, shapeNames, IM_ARRAYSIZE(shapeNames)))
        {
            auto preset = static_cast<EmitterShapePreset>(shapeIdx);
            ApplyEmitterPreset(emitter, preset);
        }

        // ======================
        ImGui::SeparatorText("Emitter");

        char nameBuf[64]{};
        snprintf(nameBuf, sizeof(nameBuf), "%s", emitterName.c_str());
        if (ImGui::InputText("Name", nameBuf, IM_ARRAYSIZE(nameBuf)))
            emitter.name = Utility::ToWString(string(nameBuf));

        const char* kindNames[] = { "Particle", "Trail" };
        int kindIdx = static_cast<int>(emitter.kind);
        if (kindIdx < 0 || kindIdx >= IM_ARRAYSIZE(kindNames))
            kindIdx = 0;
        if (ImGui::Combo("Kind", &kindIdx, kindNames, IM_ARRAYSIZE(kindNames)))
            emitter.kind = static_cast<EffectEmitterKind>(kindIdx);

        const char* spaceNames[] = { "Local", "World" };
        int spaceIndex = static_cast<int>(emitter.spaceMode);
        if (spaceIndex < 0 || spaceIndex >= IM_ARRAYSIZE(spaceNames))
            spaceIndex = 1;
        if (ImGui::Combo("Space", &spaceIndex, spaceNames, IM_ARRAYSIZE(spaceNames)))
            emitter.spaceMode = static_cast<EffectSpaceMode>(spaceIndex);

        float offset[3] = { emitter.localOffset.x, emitter.localOffset.y, emitter.localOffset.z };
        if (ImGui::DragFloat3("Local Offset", offset, 0.01f))
            emitter.localOffset = _float3(offset[0], offset[1], offset[2]);

        ImGui::Checkbox("Burst", &emitter.burst);
        ImGui::SameLine();
        ImGui::DragInt("Burst Count", &emitter.burstCount, 1.f, 0, 10000);

        ImGui::DragFloat("Delay", &emitter.delay, 0.01f, 0.f, 9999.f);
        ImGui::DragFloat("Duration", &emitter.duration, 0.01f, -1.f, 9999.f);

        if (emitter.kind == EffectEmitterKind::Particle)
        {
            ImGui::SeparatorText("Spawn (Particle)");
            DrawParticleGui(emitter.particle, curveNames, curveCount);
        }
        else if (emitter.kind == EffectEmitterKind::Trail)
        {
            DrawTrailGui(emitter, curveNames, curveCount);
        }
    }

    ImGui::PopID();
#endif
}

void EffectSystem::DrawParticleGui(ParticleSpawnData& spawn, const char** curveNames, int curveCount)
{
#ifdef USE_IMGUI
    ImGui::SeparatorText("Visual Mode");

    const char* visualNames[] = { "Particle (Billboard)", "Sprite Sheet" };
    int visualIdx = (spawn.visualMode == ParticleVisualMode::SpriteSheet) ? 1 : 0;
    int prevIdx = visualIdx;

    if (ImGui::Combo("Visual", &visualIdx, visualNames, IM_ARRAYSIZE(visualNames)))
    {
        if (visualIdx == 0 && prevIdx != 0)
        {
            // SpriteSheet -> Particle
            spawn.visualMode = ParticleVisualMode::Billboard;
            spawn.sheet.enabled = false;

            spawn.spawnRate = 10.f;
            spawn.lifeMin = 0.5f;
            spawn.lifeMax = 1.0f;
            spawn.speedMin = 5.f;
            spawn.speedMax = 10.f;
            spawn.startSize = 30.f;
            spawn.endSize = 60.f;
            spawn.startColor = _float4(1.f, 1.f, 1.f, 1.f);
            spawn.endColor = _float4(1.f, 1.f, 1.f, 0.f);
            spawn.sizeCurve = EffectCurveType::Linear;
            spawn.alphaCurve = EffectCurveType::EaseOut;
            spawn.colorCurve = EffectCurveType::Linear;
        }
        else if (visualIdx == 1 && prevIdx != 1)
        {
            // Particle -> SpriteSheet
            spawn.visualMode = ParticleVisualMode::SpriteSheet;
            spawn.sheet.enabled = true;

            spawn.speedMin = 0.f;
            spawn.speedMax = 0.f;
            spawn.startSize = max(1.f, spawn.startSize);
            spawn.endSize = spawn.startSize;

            spawn.startColor.w = 1.f;
            spawn.endColor = spawn.startColor;

            spawn.sizeCurve = EffectCurveType::Linear;
            spawn.alphaCurve = EffectCurveType::Linear;
            spawn.colorCurve = EffectCurveType::Linear;

            spawn.rotSpeedMin = 0.f;
            spawn.rotSpeedMax = 0.f;
            spawn.randomStartRot = false;

            if (spawn.sheet.cols <= 0) spawn.sheet.cols = 4;
            if (spawn.sheet.rows <= 0) spawn.sheet.rows = 4;
            spawn.sheet.startFrame = 0;
            spawn.sheet.endFrame = spawn.sheet.cols * spawn.sheet.rows - 1;
            spawn.sheet.fps = 16.f;
            spawn.sheet.loop = true;
            spawn.sheet.animate = true;
        }
    }

    // ───────── Preset ─────────
    const char* presetNames[] = { "None", "Spark", "Smoke", "HitFlash", "Ring" };
    static int presetIdx = 0;
    ImGui::Combo("Preset", &presetIdx, presetNames, IM_ARRAYSIZE(presetNames));
    ImGui::SameLine();
    if (ImGui::Button("Apply Preset"))
    {
        if (presetIdx > 0)
        {
            auto preset = static_cast<EffectPresetType>(presetIdx);
            EffectUtility::ApplyPreset(spawn, preset);

            if (spawn.sheet.enabled)
                spawn.visualMode = ParticleVisualMode::SpriteSheet;
            else
                spawn.visualMode = ParticleVisualMode::Billboard;
        }
    }

    // ───────── Spawn ─────────
    ImGui::SeparatorText("Spawn");

    ImGui::DragFloat("Spawn Rate", &spawn.spawnRate, 0.1f, 0.f, 10000.f);
    ImGui::DragFloatRange2("Life (Min/Max)", &spawn.lifeMin, &spawn.lifeMax, 0.01f, 0.f, 100.f, "Min: %.2f", "Max: %.2f");
    ImGui::DragFloatRange2("Speed (Min/Max)", &spawn.speedMin, &spawn.speedMax,  0.1f, 0.f, 1000.f, "Min: %.2f", "Max: %.2f");
    ImGui::DragFloatRange2("Pos Radius (Min/Max)",   &spawn.posRadiusMin, &spawn.posRadiusMax,  0.1f, 0.f, 500.f, "Min: %.1f", "Max: %.1f");
    float baseDir[3] = { spawn.baseDir.x, spawn.baseDir.y, spawn.baseDir.z };
    if (ImGui::DragFloat3("Base Dir", baseDir, 0.01f))
        spawn.baseDir = _float3(baseDir[0], baseDir[1], baseDir[2]);

    // ───────── Direction / Rotation ─────────
    ImGui::SeparatorText("Direction / Rotation");

    const char* dirModeNames[] = { "World", "Local (Emitter Basis)" };
    int dirModeIdx = spawn.dirLocal ? 1 : 0;
    if (ImGui::Combo("Direction Space", &dirModeIdx, dirModeNames, IM_ARRAYSIZE(dirModeNames)))
        spawn.dirLocal = (dirModeIdx == 1);

    ImGui::DragFloat("Spread Angle (rad)", &spawn.spreadAng, 0.01f, 0.f, XM_PI);
    ImGui::Checkbox("Random Start Rotation", &spawn.randomStartRot);
    ImGui::DragFloatRange2("Rot Speed (Min/Max)", &spawn.rotSpeedMin, &spawn.rotSpeedMax,
        0.01f, -50.f, 50.f, "Min: %.2f", "Max: %.2f");

    ImGui::Checkbox("Velocity From Spawn Position", &spawn.velFromPos);

    // ───────── Sprite Sheet ─────────
    ImGui::SeparatorText("Sprite Sheet");

    if (spawn.visualMode == ParticleVisualMode::SpriteSheet)
    {
        spawn.sheet.enabled = true;

        int cols = spawn.sheet.cols;
        int rows = spawn.sheet.rows;

        ImGui::DragInt("Cols", &cols, 1, 1, 64);
        ImGui::DragInt("Rows", &rows, 1, 1, 64);

        if (cols < 1) cols = 1;
        if (rows < 1) rows = 1;

        spawn.sheet.cols = cols;
        spawn.sheet.rows = rows;

        const int total = cols * rows;

        int startF = spawn.sheet.startFrame;
        int endF = spawn.sheet.endFrame;

        ImGui::DragInt("Start Frame", &startF, 1, 0, total - 1);
        ImGui::DragInt("End Frame", &endF, 1, 0, total - 1);

        if (startF < 0)      startF = 0;
        if (startF >= total) startF = total - 1;
        if (endF < startF)   endF = startF;
        if (endF >= total)   endF = total - 1;

        spawn.sheet.startFrame = startF;
        spawn.sheet.endFrame = endF;

        ImGui::DragFloat("FPS", &spawn.sheet.fps, 0.1f, 0.f, 120.f);
        ImGui::Checkbox("Loop", &spawn.sheet.loop);
        ImGui::Checkbox("Animate", &spawn.sheet.animate);
    }
    else
    {
        // Particle 모드일 때는 sheet 완전 꺼두기
        spawn.sheet.enabled = false;
    }

    // ───────── Size / Color ─────────
    ImGui::SeparatorText("Size / Color");

    ImGui::DragFloat("Start Size", &spawn.startSize, 1.f, 0.f, 10000.f);
    ImGui::DragFloat("End Size", &spawn.endSize, 1.f, 0.f, 10000.f);

    float startColor[4] = { spawn.startColor.x, spawn.startColor.y, spawn.startColor.z, spawn.startColor.w };
    float endColor[4] = { spawn.endColor.x, spawn.endColor.y, spawn.endColor.z, spawn.endColor.w };

    if (ImGui::ColorEdit4("Start Color", startColor))
        spawn.startColor = _float4(startColor[0], startColor[1], startColor[2], startColor[3]);
    if (ImGui::ColorEdit4("End Color", endColor))
        spawn.endColor = _float4(endColor[0], endColor[1], endColor[2], endColor[3]);

    // ───────── Curves ─────────
    ImGui::SeparatorText("Curves");

    int sizeCurveIdx  = static_cast<int>(spawn.sizeCurve);
    int alphaCurveIdx = static_cast<int>(spawn.alphaCurve);
    int colorCurveIdx = static_cast<int>(spawn.colorCurve);
    int rateCurveIdx  = static_cast<int>(spawn.rateCurve);

    if (ImGui::Combo("Size Curve", &sizeCurveIdx, curveNames, curveCount))
        spawn.sizeCurve = static_cast<EffectCurveType>(sizeCurveIdx);
    ImGui::SameLine();
    EffectUtility::DrawCurvePreview("##SizeCurvePreview", spawn.sizeCurve);

    if (ImGui::Combo("Alpha Curve", &alphaCurveIdx, curveNames, curveCount))
        spawn.alphaCurve = static_cast<EffectCurveType>(alphaCurveIdx);
    ImGui::SameLine();
    EffectUtility::DrawCurvePreview("##AlphaCurvePreview", spawn.alphaCurve);

    if (ImGui::Combo("Color Curve", &colorCurveIdx, curveNames, curveCount))
        spawn.colorCurve = static_cast<EffectCurveType>(colorCurveIdx);
    ImGui::SameLine();
    EffectUtility::DrawCurvePreview("##ColorCurvePreview", spawn.colorCurve);

    if (ImGui::Combo("Rate Curve", &rateCurveIdx, curveNames, curveCount))
        spawn.rateCurve = static_cast<EffectCurveType>(rateCurveIdx);
    ImGui::SameLine();
    EffectUtility::DrawCurvePreview("##RateCurvePreview", spawn.rateCurve);

    // ───────── Texture ─────────
    ImGui::SeparatorText("Texture");

    string texKeyStr = Utility::ToString(spawn.texKey);
    if (texKeyStr.empty())
        ImGui::Text("TexKey: <none>");
    else
        ImGui::Text("TexKey: %s", texKeyStr.c_str());

    if (ImGui::Button("Load Texture...", ImVec2(-1, 0)))
    {
        constexpr wchar_t texFilter[] = L"Texture files (*.dds;*.png;*.tga)\0*.dds;*.png;*.tga\0All Files (*.*)\0*.*\0";

        auto maybeTex = Utility::OpenFileDialog(texFilter, L"dds;png;tga");
        if (maybeTex)
        {
            filesystem::path texPath = *maybeTex;
            wstring texKey = texPath.stem().wstring();
            TextureMeta meta{};
            meta.fullPath = texPath.wstring();
            meta.colorSpace = TextureColorSpace::sRGB;
            assets->RegisterTexture(texKey, meta);
            spawn.texKey = texKey;
        }
    }

    string manualTexKey = Utility::ToString(spawn.texKey);
    char buf[128]{};
    snprintf(buf, sizeof(buf), "%s", manualTexKey.c_str());
    if (ImGui::InputText("TexKey (manual)", buf, IM_ARRAYSIZE(buf)))
        spawn.texKey = Utility::ToWString(string(buf));
#endif
}

void EffectSystem::DrawTrailGui(EffectEmitterDesc& emitter, const char** curveNames, int curveCount)
{
#ifdef USE_IMGUI
    TrailDesc& trail = emitter.trail;

    ImGui::SeparatorText("Trail");

    if (ImGui::Button("Reset Trail Defaults"))
    {
        // 리본 자체 기본값 (지금 그대로 둬도 됨)
        trail.lifeTime = 0.35f;
        trail.widthStart = 80.f;
        trail.widthEnd = 2.f;
        trail.colorStart = _float4(1.0f, 0.8f, 1.0f, 1.0f);
        trail.colorEnd = _float4(1.0f, 0.5f, 1.0f, 0.0f);
        trail.widthCurve = EffectCurveType::EaseOut;
        trail.alphaCurve = EffectCurveType::EaseOut;
        trail.minSegDist = 8.f;
        trail.texKey = L"019";       // 리본 텍스처

        trail.shapeMode = TrailShapeMode::ArcAnalytic;
        trail.arcRadius = 400.f;
        trail.arcStartDeg = -90.f;
        trail.arcEndDeg = 180.f;
        trail.arcUseownerCenter = true;
        trail.arcCenterOffset = _float3(0.f, 60.f, 0.f);
        trail.arcPlane = TrailArcPlane::Default;
        trail.arcRotDeg = _float3(0.f, 0.f, 0.f);

        // ── 불꽃 스파크 기본값 ──
        trail.sparkEnabled = false;     // 기본은 꺼두고, GUI에서 켜게
        trail.sparkInterval = 0.01f;     // 촘촘히 찍고 싶으면 더 작게
        trail.sparkBurstCount = 1;        // 1~2 정도로 시작

        ParticleSpawnData& s = trail.spark;

        s.spawnRate = 0.f;                // spark는 SpawnBurst로만 사용
        s.lifeMin = 0.25f;
        s.lifeMax = 0.35f;

        s.speedMin = 0.f;                // 거의 제자리에서 타오르는 느낌
        s.speedMax = 40.f;               // 살짝 튀고 싶으면 20~40 정도

        s.baseDir = _float3(0.f, 1.f, 0.f);
        s.spreadAng = XM_PIDIV2;          // 위쪽 반구로

        s.startSize = 40.f;
        s.endSize = 20.f;

        s.startColor = _float4(1.0f, 0.7f, 0.3f, 1.0f);  // 오렌지 불꽃
        s.endColor = _float4(1.0f, 0.2f, 0.0f, 0.0f);

        s.sizeCurve = EffectCurveType::EaseOut;
        s.alphaCurve = EffectCurveType::EaseOut;
        s.colorCurve = EffectCurveType::Linear;
        s.rateCurve = EffectCurveType::Linear;

        // 불꽃 스프라이트 시트 사용
        s.visualMode = ParticleVisualMode::SpriteSheet;
        s.sheet.enabled = true;
        s.sheet.cols = 4;
        s.sheet.rows = 4;
        s.sheet.startFrame = 0;
        s.sheet.endFrame = 15;
        s.sheet.fps = 24.f;
        s.sheet.loop = false;
        s.sheet.animate = true;

        s.randomStartRot = true;
        s.rotSpeedMin = -5.f;
        s.rotSpeedMax = 5.f;

        // 여기 키는 네가 실제로 로드한 불꽃 텍스처 이름으로 맞춰야 함
        s.texKey = L"particle_default"; // 예: flame_sheet.dds 로드해서 등록해두면 됨
    }

    ImGui::DragFloat("Life Time", &trail.lifeTime, 0.01f, 0.01f, 10.f);
    ImGui::DragFloat("Width Start", &trail.widthStart, 0.1f, 0.f, 1000.f);
    ImGui::DragFloat("Width End", &trail.widthEnd, 0.1f, 0.f, 1000.f);
    ImGui::DragFloat("Min Segment Dist", &trail.minSegDist, 0.01f, 0.f, 100.f);

    float colStart[4] = { trail.colorStart.x, trail.colorStart.y, trail.colorStart.z, trail.colorStart.w };
    float colEnd[4] = { trail.colorEnd.x, trail.colorEnd.y, trail.colorEnd.z, trail.colorEnd.w };

    if (ImGui::ColorEdit4("Color Start", colStart))
        trail.colorStart = _float4(colStart[0], colStart[1], colStart[2], colStart[3]);
    if (ImGui::ColorEdit4("Color End", colEnd))
        trail.colorEnd = _float4(colEnd[0], colEnd[1], colEnd[2], colEnd[3]);

    int widthCurveIdx = static_cast<int>(trail.widthCurve);
    int alphaCurveIdx = static_cast<int>(trail.alphaCurve);

    ImGui::SeparatorText("Curves (Trail)");

    if (ImGui::Combo("Width Curve", &widthCurveIdx, curveNames, curveCount))
        trail.widthCurve = static_cast<EffectCurveType>(widthCurveIdx);
    ImGui::SameLine();
    EffectUtility::DrawCurvePreview("##TrailWidthCurvePreview", trail.widthCurve);

    if (ImGui::Combo("Alpha Curve", &alphaCurveIdx, curveNames, curveCount))
        trail.alphaCurve = static_cast<EffectCurveType>(alphaCurveIdx);
    ImGui::SameLine();
    EffectUtility::DrawCurvePreview("##TrailAlphaCurvePreview", trail.alphaCurve);

    ImGui::SeparatorText("Trail Texture");

    string texKeyStr = Utility::ToString(trail.texKey);
    if (texKeyStr.empty())
        ImGui::Text("TexKey: <none>");
    else
        ImGui::Text("TexKey: %s", texKeyStr.c_str());

    if (ImGui::Button("Load Texture...", ImVec2(-1, 0)))
    {
        constexpr wchar_t texFilter[] =
            L"Texture files (*.dds;*.png;*.tga)\0*.dds;*.png;*.tga\0All Files (*.*)\0*.*\0";

        auto maybeTex = Utility::OpenFileDialog(texFilter, L"dds;png;tga");
        if (maybeTex)
        {
            filesystem::path texPath = *maybeTex;
            wstring texKey = texPath.stem().wstring();

            TextureMeta meta{};
            meta.fullPath = texPath.wstring();
            meta.colorSpace = TextureColorSpace::sRGB;

            assets->RegisterTexture(texKey, meta);
            trail.texKey = texKey;
        }
    }

    string manualTexKey = Utility::ToString(trail.texKey);
    char buf[128]{};
    snprintf(buf, sizeof(buf), "%s", manualTexKey.c_str());
    if (ImGui::InputText("TexKey (manual)", buf, IM_ARRAYSIZE(buf)))
        trail.texKey = Utility::ToWString(string(buf));

    ImGui::SeparatorText("Trail Shape");

    const char* shapeNames[] = { "FollowPath", "ArcAnalytic" };
    int shapeIdx = static_cast<int>(trail.shapeMode);
    if (shapeIdx < 0 || shapeIdx >= IM_ARRAYSIZE(shapeNames))
        shapeIdx = 0;
    if (ImGui::Combo("Shape Mode", &shapeIdx, shapeNames, IM_ARRAYSIZE(shapeNames)))
        trail.shapeMode = static_cast<TrailShapeMode>(shapeIdx);

    if (trail.shapeMode == TrailShapeMode::ArcAnalytic)
    {
        const char* planeNames[] = { "Default", "Horizontal", "Vertical" };
        int planeIdx = static_cast<int>(trail.arcPlane);
        if (planeIdx < 0 || planeIdx >= IM_ARRAYSIZE(planeNames))
            planeIdx = 0;
        if (ImGui::Combo("Arc Plane", &planeIdx, planeNames, IM_ARRAYSIZE(planeNames)))
            trail.arcPlane = static_cast<TrailArcPlane>(planeIdx);

        ImGui::DragFloat("Arc Radius", &trail.arcRadius, 1.f, 0.f, 10000.f);
        ImGui::DragFloat("Arc StartDeg", &trail.arcStartDeg, 0.5f, -360.f, 360.f);
        ImGui::DragFloat("Arc EndDeg", &trail.arcEndDeg, 0.5f, -360.f, 360.f);

        ImGui::DragFloat3("Arc Rot Deg (XYZ)", &trail.arcRotDeg.x, 0.5f, -360.f, 360.f);

        ImGui::Checkbox("Use Owner Center", &trail.arcUseownerCenter);
        ImGui::DragFloat3("Center Offset", &trail.arcCenterOffset.x, 1.f);
    }

    ImGui::SeparatorText("Trail Sparks");

    bool wasEnabled = trail.sparkEnabled;
    ImGui::Checkbox("Enable Sparks", &trail.sparkEnabled);

    if (!wasEnabled && trail.sparkEnabled && trail.spark.texKey.empty())
    {
        trail.sparkInterval = 0.02f;
        trail.sparkBurstCount = 4;

        ParticleSpawnData& s = trail.spark;
        s.spawnRate = 0.f;
        s.lifeMin = 0.2f;
        s.lifeMax = 0.4f;
        s.speedMin = 8.f;
        s.speedMax = 15.f;
        s.baseDir = _float3(0.f, 1.f, 0.f);
        s.spreadAng = XM_PIDIV4;
        s.startSize = 10.f;
        s.endSize = 5.f;
        s.startColor = _float4(1.f, 1.f, 0.6f, 1.f);
        s.endColor = _float4(1.f, 0.3f, 0.1f, 0.f);
        s.sizeCurve = EffectCurveType::EaseOut;
        s.alphaCurve = EffectCurveType::EaseOut;
        s.colorCurve = EffectCurveType::Linear;
        s.rateCurve = EffectCurveType::Linear;
        s.texKey = L"trail_default";
    }

    if (trail.sparkEnabled)
    {
        ImGui::DragFloat("Spark Interval", &trail.sparkInterval, 0.001f, 0.001f, 1.f);
        ImGui::DragInt("Spark Burst Count", &trail.sparkBurstCount, 1.f, 1, 1000);

        ImGui::PushID("Spark");
        DrawParticleGui(trail.spark, curveNames, curveCount);
        ImGui::PopID();
    }

    ImGui::SeparatorText("Trail Attach");
    ImGui::Checkbox("Use Collider OBB Tip", &emitter.useColliderObbTip);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("OBB Collider의 길이축 끝 두 점을 Trail의 p0/p1로 사용");
#endif
}

void EffectSystem::ApplyEmitterCommands( EffectArchetype& effect, int removeIndex, int duplicateIndex, int moveUpIndex, int moveDownIndex)
{
#ifdef USE_IMGUI
    int emitterCount = static_cast<int>(effect.emitters.size());

    if (removeIndex >= 0 && removeIndex < emitterCount)
    {
        effect.emitters.erase(effect.emitters.begin() + removeIndex);

        for (auto& ev : effect.events)
        {
            if (ev.emitterIdx == removeIndex)
                ev.emitterIdx = -1;
            else if (ev.emitterIdx > removeIndex)
                ev.emitterIdx -= 1;
        }
    }
    else if (duplicateIndex >= 0 && duplicateIndex < emitterCount)
    {
        auto dup = effect.emitters[duplicateIndex];
        dup.name += L"_Copy";

        effect.emitters.insert(effect.emitters.begin() + duplicateIndex + 1, dup);

        for (auto& ev : effect.events)
            if (ev.emitterIdx > duplicateIndex)
                ev.emitterIdx += 1;
    }
    else if (moveUpIndex >= 1 && moveUpIndex < emitterCount)
    {
        int a = moveUpIndex - 1;
        int b = moveUpIndex;

        swap(effect.emitters[a], effect.emitters[b]);

        for (auto& ev : effect.events)
        {
            if (ev.emitterIdx == a)
                ev.emitterIdx = b;
            else if (ev.emitterIdx == b)
                ev.emitterIdx = a;
        }
    }
    else if (moveDownIndex >= 0 && moveDownIndex < emitterCount - 1)
    {
        int a = moveDownIndex;
        int b = moveDownIndex + 1;

        swap(effect.emitters[a], effect.emitters[b]);

        for (auto& ev : effect.events)
        {
            if (ev.emitterIdx == a)
                ev.emitterIdx = b;
            else if (ev.emitterIdx == b)
                ev.emitterIdx = a;
        }
    }
#endif
}

void EffectSystem::RenderEventSection(EffectArchetype& effect)
{
#ifdef USE_IMGUI
    if (effect.events.empty())
        return;

    ImGui::SeparatorText("Events (read-only)");

    ImGui::PushID("Events");
    for (size_t i = 0; i < effect.events.size(); ++i)
    {
        const auto& ev = effect.events[i];
        ImGui::PushID(static_cast<int>(i));

        const char* typeName = "Unknown";
        switch (ev.type)
        {
        case EffectEventType::SpawnEmitter: typeName = "SpawnEmitter"; break;
        case EffectEventType::StopEmitter:  typeName = "StopEmitter";  break;
        }

        ImGui::Text( "Event[%d]  t=%.2f  type=%s  emitterIdx=%d", static_cast<int>(i), ev.time, typeName,  ev.emitterIdx );

        ImGui::PopID();
    }
    ImGui::PopID();
#endif
}