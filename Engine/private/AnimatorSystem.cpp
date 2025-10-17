#include "Enginepch.h"

Handle AnimatorSystem::Create(EntityID owner, Skeleton* skeleton, const ClipTable* clips, Handle transform, const vector<string>& baseMaskBones)
{
    Handle handle = CreateComp(owner);
    AnimData& anim = *Get(handle);
    anim = {};
    anim.skeleton = skeleton;
    anim.clips = clips;
    anim.transform = transform;
    anim.boneCount = (_uint)skeleton->bones.size();

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
    {
        auto& layer = anim.layers.back();
        layer.mask.assign(anim.boneCount, baseMaskBones.empty() ? 1 : 0);
        layer.lastPos.assign(anim.boneCount, 0);
        layer.lastRot.assign(anim.boneCount, 0);
        layer.lastScale.assign(anim.boneCount, 0);

        for (const auto& boneName : baseMaskBones)
        {
            auto it = skeleton->boneNameToIdx.find(boneName);
            if (it != skeleton->boneNameToIdx.end() && it->second < anim.boneCount)
                layer.mask[it->second] = 1;
        }
    }
    return handle;
}

void AnimatorSystem::Update(float dt, TransformSystem& transformSys)
{
    ForEachAlive([&](_uint, AnimData& anim)
        {
            // 1. Layer 시간/페이드
            for (auto& layer : anim.layers)
            {
                if (!layer.isEnabled || !layer.clip || layer.isPaused) continue;

                layer.curTime += dt * layer.clip->tickPerSec * layer.playbackSpeed;

                if (layer.playType == ANIMTYPE::ONCE)
                    layer.curTime = min(layer.curTime, (float)layer.clip->duration);
                else if (layer.playType == ANIMTYPE::LOOP && layer.clip->duration > 0.f)
                    layer.curTime = fmod(layer.curTime, (float)layer.clip->duration);

                if (layer.fadeTime > 0.f) {
                    layer.fadeElapsed = min(layer.fadeElapsed + dt, layer.fadeTime);
                    float t = Utility::Saturate(layer.fadeElapsed / layer.fadeTime);
                    layer.blendWeight = (1.f - t) * layer.blendWeight + t * layer.targetWeight;
                    if (layer.fadeElapsed >= layer.fadeTime) layer.blendWeight = layer.targetWeight;
                }
            }

            BaseSRT(anim);
            anim.blendScale = anim.baseScale;
            anim.blendRot = anim.baseRot;
            anim.blendTrans = anim.baseTrans;

            BlendSRT(anim);
            BuildLocalSRT(anim);

            if (const auto world = transformSys.GetWorld(anim.transform))
                SetFinalMatrices(anim, *world);
        });
}

const AnimClip* AnimatorSystem::FindClip(const AnimData& anim, const wstring& clipName) const
{
    if (!anim.clips) return nullptr;
    auto it = anim.clips->find(clipName);
    return (it == anim.clips->end()) ? nullptr : it->second;
}

void AnimatorSystem::Play(Handle handle, _uint layerIdx, const wstring& clipName, ANIMTYPE type)
{
    auto* anim = Get(handle);
    if (!anim || layerIdx >= anim->layers.size()) return;

    const AnimClip* clip = FindClip(*anim, clipName);
    if (!clip) return;

    auto& layer = anim->layers[layerIdx];
    layer.clip = clip;
    layer.curTime = 0.f;
    layer.playType = type;
    layer.isPaused = false;
    layer.fadeTime = 0.f;
    layer.fadeElapsed = 0.f;
    layer.targetWeight = layer.blendWeight;
}

void AnimatorSystem::PlayFade(Handle handle, _uint layerIdx, const wstring& clipName, float fadeSec, float targetWeight, ANIMTYPE type)
{
    auto* anim = Get(handle);
    if (!anim || layerIdx >= anim->layers.size()) return;

    const AnimClip* clip = FindClip(*anim, clipName);
    if (!clip) return;

    auto& layer        = anim->layers[layerIdx];
    layer.clip         = clip;
    layer.curTime      = 0.f;
    layer.playType     = type;
    layer.isPaused     = false;
    layer.fadeTime     = max(0.f, fadeSec);
    layer.fadeElapsed  = 0.f;
    layer.targetWeight = Utility::Saturate(targetWeight);
    if (layer.fadeTime <= 0.f)
        layer.blendWeight = layer.targetWeight;
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
    auto& layer       = anim->layers[layerIdx];
    layer.clip        = nullptr;
    layer.curTime     = 0.f;
    layer.isPaused    = false;
    layer.fadeTime    = 0.f;
    layer.fadeElapsed = 0.f;
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
    if (!anim || layerIdx >= anim->layers.size()) 
    { 
        assert(false); 
        return;
    }
    if (layerIdx == 0)
    { 
        anim->layers[0].blendWeight = 1.f; 
        return;
    }
    anim->layers[layerIdx].blendWeight = Utility::Saturate(weight);
}

void AnimatorSystem::SetLayerBlendType(Handle handle, _uint layerIdx, ANIMBLEND type)
{
    auto* anim = Get(handle);
    if (!anim || layerIdx >= anim->layers.size()) 
    { 
        assert(false); 
        return;
    }
    if (layerIdx == 0) return;
    anim->layers[layerIdx].blendType = type;
}

void AnimatorSystem::SetPlaybackSpeed(Handle handle, _uint layerIdx, float speed)
{
    auto* anim = Get(handle);
    if (!anim || layerIdx >= anim->layers.size())
    {
        assert(false);
        return;
    }
    anim->layers[layerIdx].playbackSpeed = speed;
}

void AnimatorSystem::SetLayerEnabled(Handle handle, _uint layerIdx, bool enabled)
{
    auto* anim = Get(handle);
    if (!anim || layerIdx >= anim->layers.size()) 
    { 
        assert(false); 
        return; 
    }
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
    if (const AnimClip* clip = FindClip(*anim, clipName)) 
        return clip->duration;
    return 0.f;
}

vector<wstring> AnimatorSystem::GetClipNames(Handle handle) const
{
    vector<wstring> out;
    const auto* anim = Get(handle);
    if (!anim || !anim->clips) return out;
    out.reserve(anim->clips->size());
    for (auto& pair : *anim->clips)
        out.push_back(pair.first);
    sort(out.begin(), out.end());
    return out;
}

_uint AnimatorSystem::GetLayerCount(Handle handle) const
{
    const auto* anim = Get(handle);
    return anim ? (_uint)anim->layers.size() : 0;
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

void AnimatorSystem::BaseSRT(AnimData& anim)
{
    if (anim.layers.empty() || !anim.layers[0].clip)
    {
        for (_uint i = 0; i < anim.boneCount; ++i)
        {
            const Bone& bone = *anim.skeleton->bones[i];
            _mat bindLocal = XMLoadFloat4x4(&bone.bindLocal);
            XMMatrixDecompose(&anim.baseScale[i], &anim.baseRot[i], &anim.baseTrans[i], bindLocal);
        }
        return;
    }

    const AnimLayerData& base = anim.layers[0];
    for (_uint i = 0; i < anim.boneCount; ++i)
        SampleSRT(anim, i, base, anim.baseScale[i], anim.baseRot[i], anim.baseTrans[i]);
}


void AnimatorSystem::BlendSRT(AnimData& anim)
{
    for (size_t layerIdx = 1; layerIdx < anim.layers.size(); ++layerIdx)
    {
        const AnimLayerData& layer = anim.layers[layerIdx];
        if (!layer.isEnabled || !layer.clip || layer.blendWeight <= 0.f) continue;

        for (_uint i = 0; i < anim.boneCount; ++i)
        {
            if (!layer.mask.empty() && layer.mask[i] == 0) continue;

            _vec scale, rot, trans;
            SampleSRT(anim, i, layer, scale, rot, trans);

            if (layer.blendType == ANIMBLEND::OVERRIDE)
            {
                anim.blendScale[i] = XMVectorLerp(anim.blendScale[i], scale, layer.blendWeight);
                anim.blendTrans[i] = XMVectorLerp(anim.blendTrans[i], trans, layer.blendWeight);
                anim.blendRot[i] = XMQuaternionSlerp(anim.blendRot[i], rot, layer.blendWeight);
            }
            else
            {
                _vec deltaScale, deltaRot, deltaTrans;
                SampleAddDelta(anim, i, layer, deltaScale, deltaRot, deltaTrans);
                anim.blendScale[i] += deltaScale * layer.blendWeight;
                anim.blendTrans[i] += deltaTrans * layer.blendWeight;

                _vec deltaQuat = XMQuaternionSlerp(XMQuaternionIdentity(), deltaRot, layer.blendWeight);
                anim.blendRot[i] = XMQuaternionNormalize(XMQuaternionMultiply(deltaQuat, anim.blendRot[i]));
            }
        }
    }
}

void AnimatorSystem::BuildLocalSRT(AnimData& anim)
{
    for (_uint i = 0; i < anim.boneCount; ++i)
    {
        _mat animMat = XMMatrixScalingFromVector(anim.blendScale[i])
                     * XMMatrixRotationQuaternion(anim.blendRot[i])
                     * XMMatrixTranslationFromVector(anim.blendTrans[i]);
        XMStoreFloat4x4(&anim.skeleton->bones[i]->animatedLocalTransform, animMat);
    }
}

void AnimatorSystem::SetFinalMatrices(AnimData& anim, const _float4x4& world)
{
    anim.skeleton->rootBone->Update(XMMatrixIdentity());

    for (_uint i = 0; i < anim.boneCount; ++i)
    {
        const Bone* bone = anim.skeleton->bonesByIdx[i];
        _mat combined = XMLoadFloat4x4(&bone->combinedTransform);
        _mat invBind = XMLoadFloat4x4(&bone->invBindPose);
        XMStoreFloat4x4(&anim.finalMatrices[i], invBind * combined);
    }
}

void AnimatorSystem::SampleSRT(const AnimData& anim, _uint boneIdx, const AnimLayerData& layer, _vec& scale, _vec& rot, _vec& trans) const
{
    SampleSRTAt(anim, boneIdx, layer, layer.curTime, scale, rot, trans);
}

void AnimatorSystem::SampleSRTAt(const AnimData& anim, _uint boneIdx, const AnimLayerData& layer, float time, _vec& scale, _vec& rot, _vec& trans) const
{
    const Bone& bone = *anim.skeleton->bones[boneIdx];

    auto it = layer.clip->boneAnims.find(bone.name);
    if (it == layer.clip->boneAnims.end())
    {
        _mat bindLocal = XMLoadFloat4x4(&bone.bindLocal);
        XMMatrixDecompose(&scale, &rot, &trans, bindLocal);
        return;
    }

    const BoneAnim& boneAnim = it->second;

    auto posIdx   = FindKeyInterval(boneAnim.posKeys, time, layer.lastPos[boneIdx]);
    auto rotIdx   = FindKeyInterval(boneAnim.rotKeys, time, layer.lastRot[boneIdx]);
    auto scaleIdx = FindKeyInterval(boneAnim.scaleKeys, time, layer.lastScale[boneIdx]);

    // cache Update
    const_cast<AnimLayerData&>(layer).lastPos[boneIdx] = posIdx.first;
    const_cast<AnimLayerData&>(layer).lastRot[boneIdx] = rotIdx.first;
    const_cast<AnimLayerData&>(layer).lastScale[boneIdx] = scaleIdx.first;

    // Pos
    if (posIdx.first == posIdx.second)
        trans = XMLoadFloat3(&boneAnim.posKeys[posIdx.first].value);
    else
    {
        const auto& key1 = boneAnim.posKeys[posIdx.first];
        const auto& key2 = boneAnim.posKeys[posIdx.second];
        const float t = (key2.time == key1.time) ? 0.f : (time - key1.time) / (key2.time - key1.time);
        trans = XMVectorLerp(XMLoadFloat3(&key1.value), XMLoadFloat3(&key2.value), t);
    }
    // Rot
    if (rotIdx.first == rotIdx.second)
        rot = XMLoadFloat4(&boneAnim.rotKeys[rotIdx.first].value);
    else
    {
        const auto& key1 = boneAnim.rotKeys[rotIdx.first];
        const auto& key2 = boneAnim.rotKeys[rotIdx.second];
        const float t = (key2.time == key1.time) ? 0.f : (time - key1.time) / (key2.time - key1.time);

        _vec quat1 = XMLoadFloat4(&key1.value);
        _vec quat2 = XMLoadFloat4(&key2.value);

        if (XMVectorGetX(XMVector4Dot(quat1, quat2)) < 0)
            quat2 = XMVectorNegate(quat2);
        rot = XMQuaternionSlerp(quat1, quat2, t);
    }
    // Scale
    if (scaleIdx.first == scaleIdx.second)
        scale = XMLoadFloat3(&boneAnim.scaleKeys[scaleIdx.first].value);
    else
    {
        const auto& key1 = boneAnim.scaleKeys[scaleIdx.first];
        const auto& key2 = boneAnim.scaleKeys[scaleIdx.second];
        const float t = (key2.time == key1.time) ? 0.f : (time - key1.time) / (key2.time - key1.time);
        scale = XMVectorLerp(XMLoadFloat3(&key1.value), XMLoadFloat3(&key2.value), t);
    }
}

void AnimatorSystem::SampleAddDelta(const AnimData& anim, _uint boneIdx, const AnimLayerData& layer, _vec& deltaScale, _vec& deltaRot, _vec& deltaTrans) const
{
    _vec curScale, curRot, curTrans;
    _vec refScale, refRot, refTrans;

    SampleSRT(anim, boneIdx, layer, curScale, curRot, curTrans);
    SampleSRTAt(anim, boneIdx, layer, 0.f, refScale, refRot, refTrans);

    deltaScale = curScale - refScale;
    deltaTrans = curTrans - refTrans;

    _vec invRef = XMQuaternionInverse(refRot);
    deltaRot = XMQuaternionNormalize(XMQuaternionMultiply(curRot, invRef));
}

void AnimatorSystem::RenderGui(EntityID id)
{
#ifdef USE_IMGUI
    ForEachOwned(id, [&](Handle handle, AnimData& anim)
        {
            ImGui::PushID((int)handle.idx);

            if (ImGui::CollapsingHeader("Animator"))
            {
                ImGui::Text("Bones: %u", anim.boneCount);
                const _uint layerCount = (_uint)anim.layers.size();

                // 레이어 추가
                static char newMaskFromClip[128] = {};
                ImGui::Separator();
                if (ImGui::SmallButton("Add Layer"))
                    AddLayer(handle, {});

                ImGui::SameLine();
                ImGui::SetNextItemWidth(160.f);
                ImGui::InputText("Mask from clip (name)", newMaskFromClip, IM_ARRAYSIZE(newMaskFromClip));
                ImGui::SameLine();
                if (ImGui::SmallButton("Add Layer (mask)"))
                {
                    const auto mask = BuildMaskFromClip(handle, Utility::ToWString(newMaskFromClip), true);
                    AddLayer(handle, {});
                    const _uint newIdx = (_uint)anim.layers.size() - 1;
                    SetLayerMask(handle, newIdx, mask);
                }
                ImGui::Separator();

                // 레이어들
                for (_uint i = 0; i < layerCount; ++i)
                {
                    ImGui::PushID((int)i);
                    auto& layer = anim.layers[i];

                    char header[64];
                    sprintf_s(header, "Layer %u%s", i, (i == 0 ? " (Base)" : ""));
                    const bool open = ImGui::TreeNodeEx(header, ImGuiTreeNodeFlags_DefaultOpen);

                    ImGui::SameLine();
                    const char* clipLabel = layer.clip ? layer.clip->name.c_str() : "(no clip)";
                    ImGui::TextDisabled("  %s | w=%.2f", clipLabel, layer.blendWeight);

                    if (open)
                    {
                        // Enable / Pause / Reset
                        bool enabled = layer.isEnabled;
                        if (ImGui::Checkbox("Enabled", &enabled))
                            SetLayerEnabled(handle, i, enabled);

                        ImGui::SameLine();
                        if (layer.isPaused)
                        {
                            if (ImGui::SmallButton("Resume")) 
                                Pause(handle, i, true);
                        }
                        else
                        {
                            if (ImGui::SmallButton("Pause"))
                                Pause(handle, i, true);
                        }
                        ImGui::SameLine();
                        if (ImGui::SmallButton("Reset"))
                            Reset(handle, i);

                        // 클립 선택
                        {
                            auto names = GetClipNames(handle);
                            int selected = -1;

                            if (layer.clip)
                            {
                                const wstring wCur = Utility::ToWString(layer.clip->name);
                                for (int k = 0; k < (int)names.size(); ++k)
                                {
                                    if (names[k] == wCur)
                                    {
                                        selected = k;
                                        break; 
                                    }
                                }
                            }

                            ImGui::SeparatorText("Clip");
                            const char* cur = (selected >= 0) ? Utility::ToString(names[selected]).c_str() : "(none)";
                            if (ImGui::BeginCombo("Clip Name", cur))
                            {
                                for (int k = 0; k < (int)names.size(); ++k)
                                {
                                    const bool isSelected = (k == selected);
                                    const string item = Utility::ToString(names[k]);
                                    if (ImGui::Selectable(item.c_str(), isSelected))
                                    {
                                        Play(handle, i, names[k], layer.playType);
                                        selected = k;
                                    }
                                    if (isSelected)
                                        ImGui::SetItemDefaultFocus();
                                }
                                ImGui::EndCombo();
                            }

                            // PlayType
                            static const char* playTypes[] = { "Loop", "Once" };
                            int playIdx = (layer.playType == ANIMTYPE::ONCE) ? 1 : 0;
                            if (ImGui::Combo("Play Type", &playIdx, playTypes, IM_ARRAYSIZE(playTypes)))
                                layer.playType = (playIdx == 1) ? ANIMTYPE::ONCE : ANIMTYPE::LOOP;

                            // Speed
                            float speed = layer.playbackSpeed;
                            if (ImGui::DragFloat("Speed", &speed, 0.01f, 0.0f, 10.0f))
                                SetPlaybackSpeed(handle, i, max(0.f, speed));

                            // Time
                            float duration = (layer.clip ? (float)layer.clip->duration : 0.f);
                            float t = layer.curTime;
                            if (ImGui::SliderFloat("Time (ticks)", &t, 0.f, max(0.001f, duration)))
                                SetLayerTime(handle, i, t);

                            // Fade / Weight
                            ImGui::SeparatorText("Fade / Weight");
                            float target = layer.targetWeight;
                            float fade = layer.fadeTime;
                            if (i == 0)
                            {
                                ImGui::BeginDisabled();
                                ImGui::SliderFloat("Weight", &layer.blendWeight, 0.f, 1.f);
                                ImGui::EndDisabled();
                                ImGui::TextDisabled("Base layer weight is fixed to 1");
                            }
                            else
                            {
                                float weight = layer.blendWeight;
                                if (ImGui::SliderFloat("Weight", &weight, 0.f, 1.f))
                                    SetLayerBlendWeight(handle, i, weight);

                                if (ImGui::DragFloat("Fade Time (sec)", &fade, 0.01f, 0.f, 10.f))
                                    layer.fadeTime = max(0.f, fade);
                                if (ImGui::DragFloat("Target Weight", &target, 0.01f, 0.f, 1.f))
                                    layer.targetWeight = Utility::Saturate(target);

                                ImGui::SameLine();
                                if (ImGui::SmallButton("Apply Fade") && layer.clip)
                                    PlayFade(handle, i, Utility::ToWString(layer.clip->name), layer.fadeTime, layer.targetWeight, layer.playType);
                            }

                            // Blend Type
                            if (i == 0)
                            {
                                ImGui::BeginDisabled();
                                static const char* blendTypes[] = { "Override", "Additive" };
                                int tmp = 0;
                                ImGui::Combo("Blend", &tmp, blendTypes, IM_ARRAYSIZE(blendTypes));
                                ImGui::EndDisabled();
                            }
                            else
                            {
                                static const char* blendTypes[] = { "Override", "Additive" };
                                int blendIdx = (layer.blendType == ANIMBLEND::ADDITIVE) ? 1 : 0;
                                if (ImGui::Combo("Blend", &blendIdx, blendTypes, IM_ARRAYSIZE(blendTypes)))
                                    SetLayerBlendType(handle, i, (blendIdx == 1 ? ANIMBLEND::ADDITIVE : ANIMBLEND::OVERRIDE));
                            }
                        }

                        // Mask
                        ImGui::SeparatorText("Mask");
                        const size_t masked = count_if(layer.mask.begin(), layer.mask.end(), [](uint8_t b) { return b != 0; });
                        ImGui::Text("Masked Bones: %zu / %u", masked, anim.boneCount);

                        if (ImGui::SmallButton("Clear Mask")) 
                        {
                            vector<uint8_t> mask(anim.boneCount, 0);
                            SetLayerMask(handle, i, mask);
                        }
                        ImGui::SameLine();
                        if (ImGui::SmallButton("Full Mask"))
                        {
                            vector<uint8_t> mask(anim.boneCount, 1);
                            SetLayerMask(handle, i, mask);
                        }

                        ImGui::TreePop();
                    }

                    ImGui::PopID(); // layer
                }
            }
            ImGui::PopID(); // handle
        });
#endif
}
