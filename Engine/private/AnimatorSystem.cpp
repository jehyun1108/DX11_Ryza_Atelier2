#include "Enginepch.h"

template<typename T>
static inline pair<uint16_t, uint16_t> FindInterval(const vector<Keyframe<T>>& keys, float time, uint16_t hint)
{
    const uint16_t n = (uint16_t)keys.size();
    if (n <= 1) return { 0, 0 };
    if (time <= keys.front().time) return { 0, 0 };
    if (time >= keys.back().time)  return { (uint16_t)(n - 1), (uint16_t)(n - 1) };

    uint16_t i = hint;
    while (i + 1 < n && !(time <= keys[i + 1].time)) ++i;
    if (i + 1 < n && time >= keys[i].time && time <= keys[i + 1].time)
        return { i, (uint16_t)(i + 1) };

    uint16_t lo = 0, hi = n - 1;
    while (lo + 1 < hi)
    {
        uint16_t mid = (lo + hi) / 2;
        (keys[mid].time <= time) ? lo = mid : hi = mid;
    }
    return { lo, hi };
}

static inline SectionInfo ResolveSection(const AnimLayerData& layer)
{
    const float duration = static_cast<float>(layer.clip->duration);
    const float eps = 1e-4f;

    float startTicks = clamp(layer.sectionStartTicks, 0.f, duration);
    float endTicks   = (layer.sectionEndTicks >= 0.f) ? clamp(layer.sectionEndTicks, 0.f, duration)  : duration;

    if (endTicks < startTicks) swap(endTicks, startTicks);
    if (endTicks - startTicks < eps) endTicks = min(duration, startTicks + eps);

    return SectionInfo{ startTicks,  endTicks, max(eps, endTicks - startTicks) };
}

static inline float WrapInSection(float curTicks, const SectionInfo& sec)
{
    const float rel = curTicks - sec.startTicks;
    float m = fmod(rel, sec.length);
    if (m < 0.f) m += sec.length; 
    return sec.startTicks + m;
}

static inline float ClampInSection(float curTicks, const SectionInfo& sec)
{
    return clamp(curTicks, sec.startTicks, sec.endTicks);
}
// --------------------------------------------------------------------------------------------------------------------------------------
Handle AnimatorSystem::Create(EntityID owner, Skeleton* skeleton, const ClipTable* clips, Handle transform, const vector<string>& baseMaskBones)
{
    Handle   handle = CreateComp(owner);
    AnimData& anim  = *Get(handle);
    anim            = {};
    anim.skeleton   = skeleton;
    anim.clips      = clips;
    anim.transform  = transform;
    anim.boneCount  = (_uint)skeleton->bones.size();

    anim.finalMatrices.resize(anim.boneCount);
    for (auto& mat : anim.finalMatrices)
        XMStoreFloat4x4(&mat, XMMatrixIdentity());

    anim.baseScale.resize(anim.boneCount);
    anim.baseRot.resize(anim.boneCount);
    anim.baseTrans.resize(anim.boneCount);

    anim.blendScale.resize(anim.boneCount);
    anim.blendRot.resize(anim.boneCount);
    anim.blendTrans.resize(anim.boneCount);

    anim.layers.emplace_back();
    anim.layers.emplace_back();
    for (AnimLayerData* layerPtr : { &anim.layers[0], &anim.layers[1] })
    {
        auto& layer = *layerPtr;
        layer.mask.assign(anim.boneCount, 1);
        layer.lastPos.assign(anim.boneCount, 0);
        layer.lastRot.assign(anim.boneCount, 0);
        layer.lastScale.assign(anim.boneCount, 0);
        layer.blendType   = ANIMBLEND::OVERRIDE;
        layer.blendWeight = 0.f; 
    }
    anim.layers[0].blendWeight = 1.f;
    anim.cross = {};
    return handle;
}

void AnimatorSystem::CrossFade(Handle handle, _uint fromLayerIndex, _uint toLayerIndex, const wstring& toClipName, float fadeDur, ANIMTYPE type, float startNormalized, float endNormalized)
{
    auto* anim = Get(handle);
    if (!anim) return;
    if (fromLayerIndex >= anim->layers.size() || toLayerIndex >= anim->layers.size()) return;

    const AnimClip* toClip = FindClip(*anim, toClipName);
    if (!toClip) return;

    auto& fromLayer = anim->layers[fromLayerIndex];
    auto& toLayer = anim->layers[toLayerIndex];

    const float durationTicks = static_cast<float>(toClip->duration);
    startNormalized = Utility::Saturate(startNormalized);
    endNormalized   = Utility::Saturate(endNormalized);
    if (endNormalized < startNormalized)
        swap(endNormalized, startNormalized);
    if (fabsf(endNormalized - startNormalized) < 1e-5f)
        endNormalized = min(1.f, startNormalized + 1e-4f);

    toLayer.clip              = toClip;
    toLayer.playType          = type;
    toLayer.isPaused          = false;
    toLayer.blendType         = ANIMBLEND::OVERRIDE;
    toLayer.sectionStartTicks = durationTicks * startNormalized;
    toLayer.sectionEndTicks   = durationTicks * endNormalized;
    toLayer.curTime           = toLayer.sectionStartTicks;
    toLayer.blendWeight        = 0.f;

    anim->cross.isActive        = true;
    anim->cross.fromLayerIndex  = fromLayerIndex;
    anim->cross.toLayerIndex    = toLayerIndex;
    anim->cross.durationSec     = max(0.f, fadeDur);
    anim->cross.elapsedSec      = 0.f;
    anim->cross.fromStartWeight = fromLayer.blendWeight;
    anim->cross.toStartWeight   = toLayer.blendWeight;
    anim->cross.pendingSwap     = false;
    anim->cross.toClipName      = toClipName;
    anim->cross.toAnimType      = type;
}

void AnimatorSystem::TickCrossFade(AnimData& anim, float dt)
{
    if (!anim.cross.isActive && !anim.cross.pendingSwap) return;

    if (anim.cross.isActive)
    {
        anim.cross.elapsedSec += dt;
        const float t01 = (anim.cross.durationSec > 0.f) ? Utility::Saturate(anim.cross.elapsedSec / anim.cross.durationSec) : 1.f;

        auto& fromLayer = anim.layers[anim.cross.fromLayerIndex];
        auto& toLayer   = anim.layers[anim.cross.toLayerIndex];

        const float fromTarget = 0.f;
        const float toTarget   = 1.f;

        fromLayer.blendWeight = Utility::Saturate(lerp(anim.cross.fromStartWeight, 0.0f, t01));
        toLayer.blendWeight   = Utility::Saturate(lerp(anim.cross.toStartWeight, 1.0f, t01));

        NormalizeFullBodyPair(anim, anim.cross.fromLayerIndex, anim.cross.toLayerIndex);

        if (t01 >= 1.f || (fromLayer.blendWeight <= 0.001f && toLayer.blendWeight >= 0.999f))
        {
            anim.cross.isActive = false;
            anim.cross.pendingSwap = true;
        }
    }

    if (anim.cross.pendingSwap)
    {
        PromoteToBaseAndClear(anim, anim.cross.fromLayerIndex, anim.cross.toLayerIndex);
        anim.cross.pendingSwap = false;
    }
}

void AnimatorSystem::Update(float dt, TransformSystem& transformSys)
{
    ForEachAliveEx([&](Handle handle, EntityID owner, AnimData& anim)
        {
            for (auto& layer : anim.layers)
            {
                if (!layer.isEnabled || !layer.clip || layer.isPaused) continue;

                const float tickPerSec = max(1.f, layer.clip->tickPerSec);
                layer.curTime += dt * tickPerSec * layer.playbackSpeed;
                const SectionInfo sec = ResolveSection(layer);

                if (layer.playType == ANIMTYPE::ONCE)  layer.curTime = ClampInSection(layer.curTime, sec);
                else                                   layer.curTime = WrapInSection(layer.curTime, sec);
            }

            TickCrossFade(anim, dt);

            BaseSRT(anim);
            anim.blendScale = anim.baseScale;
            anim.blendRot   = anim.baseRot;
            anim.blendTrans = anim.baseTrans;

            BlendSRT(anim);
            BuildLocalSRT(anim);

            if (const auto world = transformSys.GetWorld(anim.transform))
                SetFinalMatrices(anim, *world);
        });
}

void AnimatorSystem::NormalizeFullBodyPair(AnimData& anim, _uint a, _uint b)
{
    if (a >= anim.layers.size() || b >= anim.layers.size()) return;

    float sum = anim.layers[a].blendWeight + anim.layers[b].blendWeight;
    if (sum > 0.0001f)
    {
        const float inv = 1.f / sum;
        anim.layers[a].blendWeight *= inv;
        anim.layers[b].blendWeight *= inv;
    }
    else
    {
        anim.layers[a].blendWeight = 1.f;
        anim.layers[b].blendWeight = 0.f;
    }
}

void AnimatorSystem::PromoteToBaseAndClear(AnimData& anim, _uint fromIdx, _uint toIdx)
{
    if (fromIdx >= anim.layers.size() || toIdx >= anim.layers.size()) return;

    auto& fromLayer = anim.layers[fromIdx];
    auto& toLayer   = anim.layers[toIdx];

    fromLayer.clip        = toLayer.clip;
    fromLayer.curTime     = toLayer.curTime;
    fromLayer.playType    = toLayer.playType;
    fromLayer.isPaused    = toLayer.isPaused;
    fromLayer.blendType   = ANIMBLEND::OVERRIDE;
    fromLayer.blendWeight = 1.f;

    fromLayer.sectionStartTicks = toLayer.sectionStartTicks;
    fromLayer.sectionEndTicks = toLayer.sectionEndTicks;

    toLayer.clip              = nullptr;
    toLayer.curTime           = 0.f;
    toLayer.isPaused          = false;
    toLayer.blendType         = ANIMBLEND::OVERRIDE;
    toLayer.blendWeight       = 0.f;
    toLayer.sectionStartTicks = 0.f;
    toLayer.sectionEndTicks   = -1.f;
}

void AnimatorSystem::PlaySection(Handle handle, _uint layerIdx, const wstring& clipName, ANIMTYPE type, float startNormalized, float endNormalized)
{
    auto* anim = Get(handle);
    if (!anim || layerIdx >= anim->layers.size()) return;

    const AnimClip* clip = FindClip(*anim, clipName);
    if (!clip) return;

    auto& layer    = anim->layers[layerIdx];
    layer.clip     = clip;
    layer.playType = type;
    layer.isPaused = false;

    const float dur = static_cast<float>(clip->duration);
    startNormalized = Utility::Saturate(startNormalized);
    endNormalized   = Utility::Saturate(endNormalized);

    if (endNormalized < startNormalized)
        swap(endNormalized, startNormalized);
    if (fabsf(endNormalized - startNormalized) < 1e-5f) 
        endNormalized = min(1.f, startNormalized + 1e-4f);

    layer.sectionStartTicks = dur * startNormalized;
    layer.sectionEndTicks   = dur * endNormalized;
    layer.curTime           = layer.sectionStartTicks;    
}

void AnimatorSystem::Play(Handle handle, _uint layerIdx, const wstring& clipName, ANIMTYPE type)
{
    auto* anim = Get(handle);
    if (!anim || layerIdx >= anim->layers.size()) return;
    const AnimClip* clip = FindClip(*anim, clipName);
    if (!clip) return;

    auto& Layer    = anim->layers[layerIdx];
    Layer.clip     = clip;
    Layer.playType = type;
    Layer.isPaused = false;
    Layer.curTime  = 0.f;

    Layer.sectionStartTicks = 0.f;
    Layer.sectionEndTicks   = -1.f;
}

const AnimClip* AnimatorSystem::FindClip(const AnimData& anim, const wstring& clipName) const
{
    if (!anim.clips) return nullptr;
    auto it = anim.clips->find(clipName);
    return (it == anim.clips->end()) ? nullptr : it->second;
}

void AnimatorSystem::BaseSRT(AnimData& anim)
{
    if (anim.layers.empty() || !anim.layers[0].clip)
    {
        for (_uint i = 0; i < anim.boneCount; ++i)
        {
            const Bone& bone = *anim.skeleton->bones[i];
            _mat bindLocal   = XMLoadFloat4x4(&bone.bindLocal);
            XMMatrixDecompose(&anim.baseScale[i], &anim.baseRot[i], &anim.baseTrans[i], bindLocal);
        }
        return;
    }

    const AnimLayerData& baseLayer = anim.layers[0];
    for (_uint i = 0; i < anim.boneCount; ++i)
        SampleSRT(anim, i, baseLayer, anim.baseScale[i], anim.baseRot[i], anim.baseTrans[i]);
}

void AnimatorSystem::BlendSRT(AnimData& anim)
{
    for (size_t layerIndex = 1; layerIndex < anim.layers.size(); ++layerIndex)
    {
        const AnimLayerData& layer = anim.layers[layerIndex];
        if (!layer.isEnabled || !layer.clip || layer.blendWeight <= 0.f) continue;

        for (_uint i = 0; i < anim.boneCount; ++i)
        {
            if (!layer.mask.empty() && layer.mask[i] == 0) continue;

            _vec sampledScale, sampledRot, sampledTrans;
            SampleSRT(anim, i, layer, sampledScale, sampledRot, sampledTrans);

            if (layer.blendType == ANIMBLEND::OVERRIDE)
            {
                anim.blendScale[i] = XMVectorLerp(anim.blendScale[i], sampledScale, layer.blendWeight);
                anim.blendTrans[i] = XMVectorLerp(anim.blendTrans[i], sampledTrans, layer.blendWeight);
                anim.blendRot[i]   = XMQuaternionSlerp(anim.blendRot[i], sampledRot, layer.blendWeight);
            }
            else 
            {
                _vec deltaScale, deltaRot, deltaTrans;
                SampleAddDelta(anim, i, layer, deltaScale, deltaRot, deltaTrans);

                anim.blendScale[i] += deltaScale * layer.blendWeight;
                anim.blendTrans[i] += deltaTrans * layer.blendWeight;

                _vec deltaQuat   = XMQuaternionSlerp(XMQuaternionIdentity(), deltaRot, layer.blendWeight);
                anim.blendRot[i] = XMQuaternionNormalize(XMQuaternionMultiply(deltaQuat, anim.blendRot[i]));
            }
        }
    }
}

void AnimatorSystem::BuildLocalSRT(AnimData& anim)
{
    for (_uint i = 0; i < anim.boneCount; ++i)
    {
        _mat local = XMMatrixScalingFromVector(anim.blendScale[i]) * 
                     XMMatrixRotationQuaternion(anim.blendRot[i])  *
                     XMMatrixTranslationFromVector(anim.blendTrans[i]);

        XMStoreFloat4x4(&anim.skeleton->bones[i]->animatedLocalTransform, local);
    }
}

void AnimatorSystem::SetFinalMatrices(AnimData& anim, const _float4x4& world)
{
    anim.skeleton->rootBone->Update(XMMatrixIdentity()); 

    for (_uint i = 0; i < anim.boneCount; ++i)
    {
        const Bone* bone = anim.skeleton->bonesByIdx[i];
        _mat combined    = XMLoadFloat4x4(&bone->combinedTransform);
        _mat invBind     = XMLoadFloat4x4(&bone->invBindPose);
        XMStoreFloat4x4(&anim.finalMatrices[i], invBind * combined);
    }
}

void AnimatorSystem::SampleSRT(const AnimData& anim, _uint boneIdx, const AnimLayerData& layer,
    _vec& outScale, _vec& outRot, _vec& outTrans) const
{
    SampleSRTAt(anim, boneIdx, layer, layer.curTime, outScale, outRot, outTrans);
}

void AnimatorSystem::SampleSRTAt(const AnimData& anim, _uint boneIdx, const AnimLayerData& layer,
    float time, _vec& outScale, _vec& outRot, _vec& outTrans) const
{
    const Bone& bone = *anim.skeleton->bones[boneIdx];

    auto it = layer.clip->boneAnims.find(bone.name);
    if (it == layer.clip->boneAnims.end())
    {
        _mat bindLocal = XMLoadFloat4x4(&bone.bindLocal);
        XMMatrixDecompose(&outScale, &outRot, &outTrans, bindLocal);
        return;
    }

    const BoneAnim& boneAnim = it->second;

    auto posIdx   = FindInterval(boneAnim.posKeys,   time, layer.lastPos[boneIdx]);
    auto rotIdx   = FindInterval(boneAnim.rotKeys,   time, layer.lastRot[boneIdx]);
    auto scaleIdx = FindInterval(boneAnim.scaleKeys, time, layer.lastScale[boneIdx]);

    const_cast<AnimLayerData&>(layer).lastPos[boneIdx]   = posIdx.first;
    const_cast<AnimLayerData&>(layer).lastRot[boneIdx]   = rotIdx.first;
    const_cast<AnimLayerData&>(layer).lastScale[boneIdx] = scaleIdx.first;

    // Pos
    if (posIdx.first == posIdx.second)
        outTrans = XMLoadFloat3(&boneAnim.posKeys[posIdx.first].value);
    else
    {
        const auto& k1 = boneAnim.posKeys[posIdx.first];
        const auto& k2 = boneAnim.posKeys[posIdx.second];
        const float t = (k2.time == k1.time) ? 0.f : (time - k1.time) / (k2.time - k1.time);
        outTrans = XMVectorLerp(XMLoadFloat3(&k1.value), XMLoadFloat3(&k2.value), t);
    }
    // Rot 
    if (rotIdx.first == rotIdx.second)
        outRot = XMLoadFloat4(&boneAnim.rotKeys[rotIdx.first].value);
    else
    {
        const auto& k1 = boneAnim.rotKeys[rotIdx.first];
        const auto& k2 = boneAnim.rotKeys[rotIdx.second];
        const float t = (k2.time == k1.time) ? 0.f : (time - k1.time) / (k2.time - k1.time);

        _vec q1 = XMLoadFloat4(&k1.value);
        _vec q2 = XMLoadFloat4(&k2.value);
        if (XMVectorGetX(XMVector4Dot(q1, q2)) < 0) q2 = XMVectorNegate(q2);
        outRot = XMQuaternionSlerp(q1, q2, t);
    }
    // Scale
    if (scaleIdx.first == scaleIdx.second)
        outScale = XMLoadFloat3(&boneAnim.scaleKeys[scaleIdx.first].value);
    else
    {
        const auto& k1 = boneAnim.scaleKeys[scaleIdx.first];
        const auto& k2 = boneAnim.scaleKeys[scaleIdx.second];
        const float t = (k2.time == k1.time) ? 0.f : (time - k1.time) / (k2.time - k1.time);
        outScale = XMVectorLerp(XMLoadFloat3(&k1.value), XMLoadFloat3(&k2.value), t);
    }
}

void AnimatorSystem::SampleAddDelta(const AnimData& anim, _uint boneIdx, const AnimLayerData& layer,
    _vec& deltaScale, _vec& deltaRot, _vec& deltaTrans) const
{
    _vec curS, curR, curT;
    _vec refS, refR, refT;

    SampleSRT(  anim, boneIdx, layer,      curS, curR, curT);
    SampleSRTAt(anim, boneIdx, layer, 0.f, refS, refR, refT);

    deltaScale = curS - refS;
    deltaTrans = curT - refT;

    _vec invRef = XMQuaternionInverse(refR);
    deltaRot   = XMQuaternionNormalize(XMQuaternionMultiply(curR, invRef));
}

float AnimatorSystem::GetNormalizedTime(Handle handle, _uint layerIdx) const
{
    const AnimData* anim = Get(handle);
    if (!anim || layerIdx >= anim->layers.size()) return 0.f;
    const AnimLayerData& layer = anim->layers[layerIdx];
    if (!layer.clip) return 0.f;

    const SectionInfo sec = ResolveSection(layer);
    const float clamped   = clamp(layer.curTime, sec.startTicks, sec.endTicks);
    const float relTicks  = clamped - sec.startTicks;
    return (sec.length > 0.f) ? (relTicks / sec.length) : 0.f;
}

float AnimatorSystem::GetRemainingTime(Handle handle, _uint layerIdx) const
{
    const AnimData* anim = Get(handle);
    if (!anim || layerIdx >= anim->layers.size()) return 0.f;

    const AnimLayerData& layer = anim->layers[layerIdx];
    if (!layer.clip) return 0.f;

    const float       tps = max(1.f, layer.clip->tickPerSec);
    const SectionInfo sec = ResolveSection(layer);

    const float cur = clamp(layer.curTime, sec.startTicks, sec.endTicks);

    const float remainingTicks = (layer.playType == ANIMTYPE::LOOP) ? (sec.length - fmod(max(0.f, cur - sec.startTicks), sec.length))
                                                                    : (max(0.f, sec.endTicks - cur));
    return remainingTicks / tps;
}

float AnimatorSystem::GetRemainingNormalized(Handle handle, _uint layerIdx) const
{
    const AnimData* anim = Get(handle);
    if (!anim || layerIdx >= anim->layers.size()) return 0.f;

    const AnimLayerData& layer = anim->layers[layerIdx];
    if (!layer.clip) return 0.f;

    const SectionInfo sec = ResolveSection(layer);
    const float       cur = clamp(layer.curTime, sec.startTicks, sec.endTicks);
    const float remaining = (layer.playType == ANIMTYPE::LOOP) ? (sec.length - fmod(max(0.f, cur - sec.startTicks), sec.length)): (max(0.f, sec.endTicks - cur));

    return Utility::Saturate((sec.length > 0.f) ? (remaining / sec.length) : 0.f);
}

bool AnimatorSystem::IsPlaying(Handle handle, _uint layerIdx) const
{
    const AnimData* anim = Get(handle);
    if (!anim || layerIdx >= anim->layers.size()) return false;
    const AnimLayerData& layer = anim->layers[layerIdx];
    return (layer.isEnabled && !layer.isPaused && layer.clip != nullptr);
}

bool AnimatorSystem::IsPlayingClip(Handle handle, _uint layerIdx, const wstring& clipName) const
{
    const AnimData* anim = Get(handle);
    if (!anim || layerIdx >= anim->layers.size()) return false;

    const AnimLayerData& layer = anim->layers[layerIdx];
    if (!layer.clip) return false;

    const AnimClip* target = FindClip(*anim, clipName);
    return (target && target == layer.clip);
}

bool AnimatorSystem::IsCrossFading(Handle handle) const
{
    const auto anim = Get(handle);
    if (!anim) return false;
    return anim->cross.isActive;
}

void AnimatorSystem::Pause(Handle handle, _uint layerIdx, bool toggle)
{
    auto* anim = Get(handle);
    if (!anim || layerIdx >= anim->layers.size()) return;

    auto& layer = anim->layers[layerIdx];
    if (!layer.clip) return;
    layer.isPaused = toggle ? !layer.isPaused : true;
}

void AnimatorSystem::Reset(Handle handle, _uint layerIdx)
{
    auto* anim = Get(handle);
    if (!anim || layerIdx >= anim->layers.size()) return;

    auto& layer     = anim->layers[layerIdx];
    layer.clip      = nullptr;
    layer.curTime   = 0.f;
    layer.isPaused  = false;
    layer.blendType = ANIMBLEND::OVERRIDE;

    layer.sectionStartTicks = 0.f;
    layer.sectionEndTicks   = -1.f;
}

void AnimatorSystem::AddLayer(Handle handle, const vector<string>& maskedBoneNames)
{
    auto* anim = Get(handle);
    if (!anim) return;

    anim->layers.emplace_back();
    auto& layer = anim->layers.back();

    layer.mask.assign(anim->boneCount, maskedBoneNames.empty() ? 1 : 0);
    layer.lastPos.assign(anim->boneCount, 0);
    layer.lastRot.assign(anim->boneCount, 0);
    layer.lastScale.assign(anim->boneCount, 0);
    layer.blendType   = ANIMBLEND::OVERRIDE;
    layer.blendWeight = 0.f;

    for (const auto& name : maskedBoneNames)
    {
        auto it = anim->skeleton->boneNameToIdx.find(name);
        if (it != anim->skeleton->boneNameToIdx.end() && it->second < anim->boneCount)
            layer.mask[it->second] = 1;
    }
}

void AnimatorSystem::SetLayerBlendWeight(Handle handle, _uint layerIdx, float weight)
{
    auto* anim = Get(handle);
    if (!anim || layerIdx >= anim->layers.size()) { assert(false); return; }

    if (layerIdx == 0) { anim->layers[0].blendWeight = 1.f; return; }
    anim->layers[layerIdx].blendWeight = Utility::Saturate(weight);
}

void AnimatorSystem::SetLayerBlendType(Handle handle, _uint layerIdx, ANIMBLEND type)
{
    auto* anim = Get(handle);
    if (!anim || layerIdx >= anim->layers.size()) { assert(false); return; }
    if (layerIdx == 0) return;
    anim->layers[layerIdx].blendType = type;
}

void AnimatorSystem::SetPlaybackSpeed(Handle handle, _uint layerIdx, float speed)
{
    auto* anim = Get(handle);
    if (!anim || layerIdx >= anim->layers.size()) { assert(false); return; }
    anim->layers[layerIdx].playbackSpeed = speed;
}

void AnimatorSystem::SetLayerEnabled(Handle handle, _uint layerIdx, bool enabled)
{
    auto* anim = Get(handle);
    if (!anim || layerIdx >= anim->layers.size()) { assert(false); return; }
    anim->layers[layerIdx].isEnabled = enabled;
}

void AnimatorSystem::SetLayerTime(Handle handle, _uint layerIdx, float tick)
{
    auto* anim = Get(handle);
    if (!anim || layerIdx >= anim->layers.size()) return;

    auto& layer = anim->layers[layerIdx];
    if (!layer.clip) return;

    const float dur = max(0.f, (float)layer.clip->duration);
    layer.curTime = clamp(tick, 0.f, dur);
}

void AnimatorSystem::SetLayerMask(Handle handle, _uint layerIdx, const vector<uint8_t>& mask)
{
    auto* anim = Get(handle);
    if (!anim || layerIdx >= anim->layers.size()) return;

    auto& layer = anim->layers[layerIdx];
    layer.mask.assign(anim->boneCount, 0);
    for (_uint i = 0; i < anim->boneCount && i < mask.size(); ++i)
        layer.mask[i] = mask[i] ? 1 : 0;
}

float AnimatorSystem::GetClipDuration(Handle handle, const wstring& clipName) const
{
    const auto* anim = Get(handle);
    if (!anim) return 0.f;
    if (const AnimClip* clip = FindClip(*anim, clipName)) return (float)clip->duration;
    return 0.f;
}

vector<wstring> AnimatorSystem::GetClipNames(Handle handle) const
{
    vector<wstring> names;
    const auto* anim = Get(handle);
    if (!anim || !anim->clips) return names;

    names.reserve(anim->clips->size());
    for (auto& p : *anim->clips) names.push_back(p.first);
    sort(names.begin(), names.end());
    return names;
}

_uint AnimatorSystem::GetLayerCount(Handle handle) const
{
    const auto* anim = Get(handle);
    return anim ? (_uint)anim->layers.size() : 0;
}

const _float4x4* AnimatorSystem::GetBoneWorld(Handle handle, _uint boneIdx) const
{
    const auto* anim = Get(handle);
    if (!anim || !anim->skeleton) return nullptr;
    if (boneIdx >= anim->boneCount) return nullptr;
    return &anim->skeleton->bonesByIdx[boneIdx]->combinedTransform;
}

_uint AnimatorSystem::GetBoneIdxByName(Handle handle, const string& boneName) const
{
    const auto* anim = Get(handle);
    if (!anim) return (_uint)-1;
    auto it = anim->skeleton->boneNameToIdx.find(boneName);
    return (it == anim->skeleton->boneNameToIdx.end()) ? (_uint)-1 : it->second;
}

const vector<_float4x4>* AnimatorSystem::GetFinalMatrices(Handle handle) const
{
    const auto* anim = Get(handle);
    return anim ? &anim->finalMatrices : nullptr;
}

vector<uint8_t> AnimatorSystem::BuildMaskFromClip(Handle handle, const wstring& clipName, bool includeChildren) const
{
    vector<uint8_t> mask;
    const auto* anim = Get(handle);
    if (!anim || !anim->clips || !anim->skeleton) return mask;

    mask.assign(anim->boneCount, 0);
    const AnimClip* clip = FindClip(*anim, clipName);
    if (!clip) return mask;

    for (const auto& pair : clip->boneAnims)
    {
        auto it = anim->skeleton->boneNameToIdx.find(pair.first);
        if (it != anim->skeleton->boneNameToIdx.end() && it->second < anim->boneCount)
            mask[it->second] = 1;
    }

    if (includeChildren)
    {
        vector<uint8_t> visited(anim->boneCount, 0);
        function<void(_uint)> dfs = [&](auto i)
            {
                if (i >= anim->boneCount || visited[i]) return;
                visited[i] = 1;  mask[i] = 1;
                const Bone* bone = anim->skeleton->bonesByIdx[i];
                for (auto child : bone->children)
                {
                    auto it = anim->skeleton->boneNameToIdx.find(child->name);
                    if (it != anim->skeleton->boneNameToIdx.end())
                        dfs(it->second);
                }
            };
        for (_uint i = 0; i < anim->boneCount; ++i)
            if (mask[i]) dfs(i);
    }
    return mask;
}

void AnimatorSystem::RenderGui(EntityID id)
{
#ifdef USE_IMGUI
    ForEachOwned(id, [&](Handle animHandle, AnimData& anim)
        {
            ImGui::PushID((int)animHandle.idx);

            if (!ImGui::CollapsingHeader("Animator")) { ImGui::PopID(); return; }

            ImGui::Text("Bones: %u", anim.boneCount);
            const uint32_t totalLayerCount = static_cast<uint32_t>(anim.layers.size());
            ImGui::SeparatorText("CrossFade");

            const bool hasAtLeastTwoLayers = (totalLayerCount >= 2);
            if (!hasAtLeastTwoLayers)
                ImGui::TextDisabled("Need at least 2 layers to crossfade.");
            else
            {
                const uint32_t safeFrom = (anim.cross.fromLayerIndex < totalLayerCount) ? anim.cross.fromLayerIndex : 0;
                const uint32_t safeTo = (anim.cross.toLayerIndex < totalLayerCount) ? anim.cross.toLayerIndex : 1;

                const bool crossFading = anim.cross.isActive || anim.cross.pendingSwap;
                const float duration = max(0.0f, anim.cross.durationSec);
                const float tClamped = (duration > 0.0f) ? Utility::Saturate(anim.cross.elapsedSec / duration) : (crossFading ? 1.0f : 0.0f);

                ImGui::Text("Status: %s (t=%.2f / %.2f, %.0f%%)",
                    anim.cross.isActive ? "Fading" : (anim.cross.pendingSwap ? "Promoting" : "Idle"),
                    anim.cross.elapsedSec, duration, tClamped * 100.0f);

                const AnimLayerData* fromLayerPtr = (safeFrom < anim.layers.size()) ? &anim.layers[safeFrom] : nullptr;
                const AnimLayerData* toLayerPtr = (safeTo < anim.layers.size()) ? &anim.layers[safeTo] : nullptr;
                if (fromLayerPtr && toLayerPtr)
                {
                    ImGui::Text("From[%u] w=%.2f  ->  To[%u] w=%.2f",
                        safeFrom, fromLayerPtr->blendWeight, safeTo, toLayerPtr->blendWeight);
                }
                else
                    ImGui::TextDisabled("Layer index invalid. Crossfade disabled.");

                vector<wstring> clipNames = GetClipNames(animHandle);

                int chosenClipIndex = -1;
                if (toLayerPtr && toLayerPtr->clip)
                {
                    const wstring currentToClip = Utility::ToWString(toLayerPtr->clip->name);
                    for (int nameIndex = 0; nameIndex < static_cast<int>(clipNames.size()); ++nameIndex)
                    {
                        if (clipNames[nameIndex] == currentToClip) { chosenClipIndex = nameIndex; break; }
                    }
                }

                string currentClipLabel = (chosenClipIndex >= 0 && chosenClipIndex < static_cast<int>(clipNames.size()))
                    ? Utility::ToString(clipNames[chosenClipIndex])
                    : string("(select target clip)");
                ImGui::SetNextItemWidth(220.0f);
                if (ImGui::BeginCombo("To Clip", currentClipLabel.c_str()))
                {
                    for (int nameIndex = 0; nameIndex < static_cast<int>(clipNames.size()); ++nameIndex)
                    {
                        const bool isSelected = (nameIndex == chosenClipIndex);
                        const string itemLabel = Utility::ToString(clipNames[nameIndex]);
                        if (ImGui::Selectable(itemLabel.c_str(), isSelected))
                            chosenClipIndex = nameIndex;
                        if (isSelected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                static int toPlayTypeIndex = 1; // 0:Loop, 1:Once
                ImGui::RadioButton("Loop", &toPlayTypeIndex, 0); ImGui::SameLine();
                ImGui::RadioButton("Once", &toPlayTypeIndex, 1);

                static float crossFadeSeconds = 0.20f;
                ImGui::SetNextItemWidth(120.0f);
                ImGui::DragFloat("Fade Seconds", &crossFadeSeconds, 0.01f, 0.0f, 10.0f);

                const bool canExecuteCrossFade = (!crossFading) && (chosenClipIndex >= 0) && (chosenClipIndex < static_cast<int>(clipNames.size())) && (fromLayerPtr && toLayerPtr);
                ImGui::BeginDisabled(!canExecuteCrossFade);

                if (ImGui::SmallButton("CrossFade From->To"))
                {
                    const wstring& toClip = clipNames[static_cast<size_t>(chosenClipIndex)];
                    CrossFade(animHandle, safeFrom, safeTo, toClip, crossFadeSeconds,
                        (toPlayTypeIndex == 1) ? ANIMTYPE::ONCE : ANIMTYPE::LOOP);
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("CrossFade To->From"))
                {
                    const wstring& toClip = clipNames[static_cast<size_t>(chosenClipIndex)];
                    CrossFade(animHandle, safeTo, safeFrom, toClip, crossFadeSeconds,
                        (toPlayTypeIndex == 1) ? ANIMTYPE::ONCE : ANIMTYPE::LOOP);
                }

                ImGui::EndDisabled();

                if (crossFading)
                    ImGui::TextDisabled("CrossFade in progress: manual weight edits are disabled.");
            }

            ImGui::SeparatorText("Layers");

            if (ImGui::SmallButton("Add Layer")) { AddLayer(animHandle, {}); }

            const uint32_t layerCountSnapshot = static_cast<uint32_t>(anim.layers.size());
            for (uint32_t layerIndex = 0; layerIndex < layerCountSnapshot; ++layerIndex)
            {
                ImGui::PushID((int)layerIndex);
                AnimLayerData& layerRef = anim.layers[layerIndex];

                char headerText[64]{};
                sprintf_s(headerText, "Layer %u%s", layerIndex, (layerIndex == 0 ? " (Base)" : ""));
                const bool nodeOpen = ImGui::TreeNodeEx(headerText, ImGuiTreeNodeFlags_DefaultOpen);

                ImGui::SameLine();
                const char* currentClipCStr = layerRef.clip ? layerRef.clip->name.c_str() : "(no clip)";
                ImGui::TextDisabled("  %s | w=%.2f", currentClipCStr, layerRef.blendWeight);

                if (nodeOpen)
                {
                    const bool isBaseLayer = (layerIndex == 0);
                    const bool crossFading = anim.cross.isActive || anim.cross.pendingSwap;

                    // Enable / Pause / Reset
                    bool newEnabled = layerRef.isEnabled;
                    if (ImGui::Checkbox("Enabled", &newEnabled))
                        SetLayerEnabled(animHandle, layerIndex, newEnabled);

                    ImGui::SameLine();
                    if (layerRef.isPaused)
                    {
                        if (ImGui::SmallButton("Resume")) Pause(animHandle, layerIndex, false); // 오타 수정 (false)
                    }
                    else
                    {
                        if (ImGui::SmallButton("Pause"))  Pause(animHandle, layerIndex, true);
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Reset"))     Reset(animHandle, layerIndex);

                    ImGui::SeparatorText("Clip");
                    vector<wstring> clipNamesForLayer = GetClipNames(animHandle);

                    int currentIndex = -1;
                    if (layerRef.clip)
                    {
                        const wstring current = Utility::ToWString(layerRef.clip->name);
                        for (int nameIndex = 0; nameIndex < static_cast<int>(clipNamesForLayer.size()); ++nameIndex)
                            if (clipNamesForLayer[nameIndex] == current) { currentIndex = nameIndex; break; }
                    }

                    string currentClipLabel = (currentIndex >= 0) ? Utility::ToString(clipNamesForLayer[currentIndex]) : string("(none)");
                    if (ImGui::BeginCombo("Clip Name", currentClipLabel.c_str()))
                    {
                        for (int nameIndex = 0; nameIndex < static_cast<int>(clipNamesForLayer.size()); ++nameIndex)
                        {
                            const bool isSelected = (nameIndex == currentIndex);
                            const string itemLabel = Utility::ToString(clipNamesForLayer[nameIndex]);
                            if (ImGui::Selectable(itemLabel.c_str(), isSelected))
                            {
                                Play(animHandle, layerIndex, clipNamesForLayer[nameIndex], layerRef.playType);
                                currentIndex = nameIndex;
                            }
                            if (isSelected) ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }

                    static const char* playTypeLabels[] = { "Loop", "Once" };
                    int playTypeIndex = (layerRef.playType == ANIMTYPE::ONCE) ? 1 : 0;
                    if (ImGui::Combo("Play Type", &playTypeIndex, playTypeLabels, IM_ARRAYSIZE(playTypeLabels)))
                        layerRef.playType = (playTypeIndex == 1) ? ANIMTYPE::ONCE : ANIMTYPE::LOOP;

                    float newPlaybackSpeed = layerRef.playbackSpeed;
                    if (ImGui::DragFloat("Speed", &newPlaybackSpeed, 0.01f, 0.0f, 10.0f))
                        SetPlaybackSpeed(animHandle, layerIndex, max(0.0f, newPlaybackSpeed));

                    const float clipDurationTicks = layerRef.clip ? static_cast<float>(layerRef.clip->duration) : 0.0f;
                    float newTimeTicks = layerRef.curTime;
                    if (ImGui::SliderFloat("Time (ticks)", &newTimeTicks, 0.0f, max(0.001f, clipDurationTicks)))
                        SetLayerTime(animHandle, layerIndex, newTimeTicks);

                    ImGui::SeparatorText("Weight / Blend");
                    if (isBaseLayer)
                    {
                        ImGui::BeginDisabled();
                        float baseWeightView = layerRef.blendWeight;
                        ImGui::SliderFloat("Weight", &baseWeightView, 0.0f, 1.0f);
                        ImGui::EndDisabled();
                        ImGui::TextDisabled("Base layer weight is fixed to 1.0");
                    }
                    else
                    {
                        ImGui::BeginDisabled(crossFading);
                        float newWeight = layerRef.blendWeight;
                        if (ImGui::SliderFloat("Weight", &newWeight, 0.0f, 1.0f))
                            SetLayerBlendWeight(animHandle, layerIndex, newWeight);
                        ImGui::EndDisabled();

                        static const char* blendTypeLabels[] = { "Override", "Additive" };
                        int blendIndex = (layerRef.blendType == ANIMBLEND::ADDITIVE) ? 1 : 0;
                        if (ImGui::Combo("Blend", &blendIndex, blendTypeLabels, IM_ARRAYSIZE(blendTypeLabels)))
                            SetLayerBlendType(animHandle, layerIndex, (blendIndex == 1 ? ANIMBLEND::ADDITIVE : ANIMBLEND::OVERRIDE));
                    }

                    ImGui::SeparatorText("Mask");
                    const size_t maskedCount = count_if(layerRef.mask.begin(), layerRef.mask.end(),
                        [](uint8_t bit) { return bit != 0; });
                    ImGui::Text("Masked Bones: %zu / %u", maskedCount, anim.boneCount);

                    if (ImGui::SmallButton("Clear Mask"))
                    {
                        vector<uint8_t> emptyMask(anim.boneCount, 0);
                        SetLayerMask(animHandle, layerIndex, emptyMask);
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Full Mask"))
                    {
                        vector<uint8_t> fullMask(anim.boneCount, 1);
                        SetLayerMask(animHandle, layerIndex, fullMask);
                    }
                    ImGui::TreePop();
                }
                ImGui::PopID(); 
            }
            ImGui::PopID(); 
        });
#endif
}
