#include "Enginepch.h"

void SkyboxSystem::OnBoot()
{
    tfSys = &registry.Get<TransformSystem>();
}

Handle SkyboxSystem::Create(EntityID owner, Handle tfHandle, const vector<SkySubmesh>& submeshList, SkyTextureType texType, bool attachToCam, float uniformScale, float baseYawRad, float rotSpeed)
{
	Handle handle      = CreateComp(owner);
	auto& state        = *Get(handle);
    state.tf           = tfHandle;
	state.enabled      = false;
    state.submeshes    = submeshList;
    state.texType      = texType;
    state.attachToCam  = attachToCam;
    state.uniformScale = uniformScale;
    state.baseYawRad   = baseYawRad;
    state.rotSpeed     = rotSpeed;
    state.phaseRad     = 0.f;
    return handle;
}

void SkyboxSystem::SetActive(Handle handle, bool enable)
{
    if (!enable)
    {
        activeHandle = {};
        return;
    }

    if (!Validate(handle))
    {
        activeHandle = {};
        return;
    }

    ForEachAliveEx([&](Handle handle, EntityID owner, SkyboxState& state) { state.enabled = false; });

    if (auto state = Get(handle))
    {
        state->enabled = true;
        activeHandle = handle;
    }
}

void SkyboxSystem::Tick(float dt)
{
    if (Validate(activeHandle))
    {
        if (auto state = Get(activeHandle))
        {
            state->phaseRad += (state->rotSpeed * dt);
            Utility::WrapToTwoPi(state->phaseRad);
        }
    }

    // CrossFade
    if (crossFade.active)
    {
        if (crossFade.dur <= 1e-6f)
            crossFade.dur = 1e-6f;
        crossFade.progress01 += (dt / crossFade.dur);
        if (crossFade.progress01 >= 1.f)
        {
            crossFade.progress01 = 1.f;
            crossFade.active = false;

            Handle toHandle{};
            SkyboxState* toState{};
            if (FindById(crossFade.toId, toHandle, toState))
                SetActive(toHandle, true);
        }
        AdvancePhase(crossFade.fromId, dt);
        AdvancePhase(crossFade.toId, dt);
    }
}

void SkyboxSystem::ExtractSkyboxProxies(SkyboxProxy& out) const
{
    out = {};
    if (!Validate(activeHandle)) return;

    if (const SkyboxState* state = Get(activeHandle))
    {
        if (!state->enabled || state->submeshes.empty()) return;

        out.enabled        = true;
        out.submeshes      = state->submeshes;  
        out.textureType    = state->texType;
        out.attachToCamera = state->attachToCam;
        out.uniformScale   = state->uniformScale;
        out.baseYawRad     = state->baseYawRad;
        out.rotSpeed       = state->rotSpeed;
        out.phaseRad       = state->phaseRad;    

        if (state->attachToCam && state->tf.IsValid())
        {
           if (const _float4x4* world = tfSys->GetWorld(state->tf))
           {
               out.hasTfYaw = true;
               out.tfYawRad = Utility::ExtractYawFromWorld(*world);
           }
        }
    }
}

void SkyboxSystem::ExtractFadeProxies(optional<SkyboxProxy>& outFrom, optional<SkyboxProxy>& outTo, float& outBlendWeight) const
{
    outFrom.reset();
    outTo.reset();
    outBlendWeight = 0.f;

    if (!crossFade.active) return;

    Handle fromHandle{}, toHandle{};
    const SkyboxState* fromState{};
    const SkyboxState* toState{};

    if (!FindById(crossFade.fromId, fromHandle, fromState)) return;
    if (!FindById(crossFade.toId, toHandle, toState)) return;
    if (!fromState || !toState) return;
    if (fromState->submeshes.empty() || toState->submeshes.empty()) return;

    SkyboxProxy fromProxy{};
    fromProxy.enabled        = true;
    fromProxy.submeshes      = fromState->submeshes;
    fromProxy.textureType    = fromState->texType;
    fromProxy.attachToCamera = fromState->attachToCam;
    fromProxy.uniformScale   = fromState->uniformScale;
    fromProxy.baseYawRad     = fromState->baseYawRad;
    fromProxy.rotSpeed       = fromState->rotSpeed;
    fromProxy.phaseRad       = fromState->phaseRad;

    SkyboxProxy toProxy{};
    toProxy.enabled        = true;
    toProxy.submeshes      = toState->submeshes;
    toProxy.textureType    = toState->texType;
    toProxy.attachToCamera = toState->attachToCam;
    toProxy.uniformScale   = toState->uniformScale;
    toProxy.baseYawRad     = toState->baseYawRad;
    toProxy.rotSpeed       = toState->rotSpeed;
    toProxy.phaseRad       = toState->phaseRad;

    outFrom = move(fromProxy);
    outTo   = move(toProxy);
    outBlendWeight = Utility::Saturate(crossFade.progress01);
}

void SkyboxSystem::SetAttachToCam(Handle handle, bool attach)
{
    auto state = Get(handle);
    state->attachToCam = attach;
}

void SkyboxSystem::SetUniformScale(Handle handle, float scale)
{
    auto state = Get(handle);
    state->uniformScale = scale;
}

void SkyboxSystem::SetBaseYaw(Handle handle, float baseYawRad)
{
    auto state = Get(handle);
    state->baseYawRad = baseYawRad;
}

void SkyboxSystem::SetRotSpeed(Handle handle, float rotSpeed)
{
    auto state = Get(handle);
    state->rotSpeed = rotSpeed;
}

void SkyboxSystem::SetPhase(Handle handle, float phaseRad)
{
    auto state = Get(handle);
    state->phaseRad = phaseRad;
    Utility::WrapToTwoPi(state->phaseRad);
}

void SkyboxSystem::SetSubmeshes(Handle handle, const vector<SkySubmesh>& submeshList)
{
    auto state = Get(handle);
    state->submeshes = submeshList;
}

void SkyboxSystem::SetTextureType(Handle handle, SkyTextureType type)
{
    auto state = Get(handle);
    state->texType = type;
}

void SkyboxSystem::StartCrossFade(Handle fromHandle, Handle toHandle, float dur)
{
    if (!Validate(fromHandle) || !Validate(toHandle)) return;

    crossFade.active     = true;
    crossFade.fromId     = GetOwner(fromHandle);
    crossFade.toId       = GetOwner(toHandle);
    crossFade.progress01 = 0.f;
    crossFade.dur        = (dur <= 0.f ? 0.001f : dur);
}

bool SkyboxSystem::FindById(EntityID owner, Handle& outHandle, const SkyboxState*& outPtr) const
{
    return pool.FindOwned(owner, outHandle, outPtr);
}

bool SkyboxSystem::FindById(EntityID owner, Handle& outHandle, SkyboxState*& outPtr)
{
    return pool.FindOwned(owner, outHandle, outPtr);
}

void SkyboxSystem::AdvancePhase(EntityID owner, float dt)
{
    Handle handle{};
    SkyboxState* state{};
    if (FindById(owner, handle, state) && state)
    {
        state->phaseRad += (state->rotSpeed * dt);
        Utility::WrapToTwoPi(state->phaseRad);
    }
}

void SkyboxSystem::RenderGui(EntityID id)
{
#ifdef USE_IMGUI
    auto draw_one = [&](Handle handle, SkyboxState& s)
        {
            const bool isActive = (Validate(activeHandle) && activeHandle.idx == handle.idx && activeHandle.generation == handle.generation);

            // Header
            ImGui::Separator();
            ImGui::Text("Owner: %u  [handle: %u:%u]%s", GetOwner(handle), handle.idx, handle.generation, isActive ? "  <ACTIVE>" : "");
            ImGui::SameLine();
            if (!isActive) 
                if (ImGui::SmallButton("Set Active")) SetActive(handle, true);
            else
            {
                ImGui::BeginDisabled();
                ImGui::SmallButton("Set Active");
                ImGui::EndDisabled();
            }

            // 활성/비활성 (참조용)
            ImGui::Text("Enabled: %s", s.enabled ? "true" : "false");
            ImGui::SameLine();
            if (ImGui::SmallButton("Disable")) SetActive({}, false);

            // Attach to camera
            bool attach = s.attachToCam;
            if (ImGui::Checkbox("Attach To Camera (snap position)", &attach)) SetAttachToCam(handle, attach);

            // Uniform scale
            float scale = s.uniformScale;
            if (ImGui::DragFloat("Uniform Scale", &scale, 1.f, 1.f, 100000.f, "%.3f"))
            {
                scale = max(1e-6f, scale);
                SetUniformScale(handle, scale);
            }

            // Texture Type
            {
                int cur = static_cast<int>(s.texType);
                const char* items[] = { "Equirect2D", "CubeMap", "Other" }; // 필요하면 enum 항목 수에 맞춰 수정
                int count = (int)std::size(items);
                if (cur < 0 || cur >= count) cur = 0;
                if (ImGui::Combo("Texture Type", &cur, items, count)) 
                    SetTextureType(handle, static_cast<SkyTextureType>(cur));
            }

            // Rotation (Yaw only)
            float baseYaw = s.baseYawRad;
            float baseYawDeg = XMConvertToDegrees(baseYaw);
            if (ImGui::DragFloat("Base Yaw (deg)", &baseYawDeg, 0.1f, -100000.f, 100000.f, "%.3f")) 
            {
                baseYaw = XMConvertToRadians(baseYawDeg);
                SetBaseYaw(handle, baseYaw);
            }

            float rotSpeed = s.rotSpeed;
            if (ImGui::DragFloat("Rot Speed (rad/s)", &rotSpeed, 0.0001f, -10.f, 10.f, "%.5f"))
                SetRotSpeed(handle, rotSpeed);

            float phase = s.phaseRad;
            float phaseDeg = XMConvertToDegrees(phase);
            if (ImGui::DragFloat("Phase (deg)", &phaseDeg, 0.1f, -100000.f, 100000.f, "%.3f")) 
            {
                phase = XMConvertToRadians(phaseDeg);
                SetPhase(handle, phase);
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Wrap 0..2PI"))
            {
                Utility::WrapToTwoPi(phase);
                SetPhase(handle, phase);
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Reset Phase")) 
                SetPhase(handle, 0.f);
            

            // TF yaw (참고값)
            {
                bool hasTf = s.attachToCam && s.tf.IsValid();
                ImGui::Text("TF Handle: %s  (%u:%u)", hasTf ? "valid" : "none", s.tf.idx, s.tf.generation);

                if (hasTf) 
                {
                        if (const _float4x4* w = tfSys->GetWorld(s.tf))
                        {
                            const float tfYaw = Utility::ExtractYawFromWorld(*w);
                            ImGui::Text("TF Yaw: %.4f rad (%.3f deg)", tfYaw, XMConvertToDegrees(tfYaw));
                            ImGui::SameLine();
                            ImGui::TextDisabled("(theta = base + phase %s tfYaw)",
                                (s.attachToCam ? "+ " : "+ (ignored: attachToCam=false) "));
                        }
                        else
                            ImGui::Text("TF world: <null>");
                }
            }

            // Submesh info (요약)
            if (ImGui::TreeNode("Submeshes"))
            {
                ImGui::Text("Count: %zu", s.submeshes.size());
                size_t idx = 0;
                for (const auto& sm : s.submeshes)
                {
                    ImGui::PushID((int)idx);
                    ImGui::Separator();
                    ImGui::Text("#%zu  mesh:%s  mat:%s  transparent:%s",
                        idx,
                        sm.mesh ? "yes" : "no",
                        sm.material ? "yes" : "no",
                        sm.transparent ? "true" : "false");
                    if (sm.mesh)
                    {
                        ImGui::BulletText("Vtx:%u  Idx:%u  Layout:%d  Topo:%d",
                            sm.mesh->GetVtxCount(), sm.mesh->GetIdxCount(),
                            (int)sm.mesh->GetLayoutID(), (int)sm.mesh->GetTopology());
                    }
                    ImGui::PopID();
                    ++idx;
                }
                ImGui::TreePop();
            }
        };
#endif
}