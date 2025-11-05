#include "Enginepch.h"

namespace
{
    inline float WrapAngleDeg(float a)
    {
        a = fmodf(a, 360.f);
        if (a < -180.f)  a += 360.f;
        if (a >= 180.f)  a -= 360.f;
        return a;
    }
}

void FreeCamSystem::OnBoot()
{
    input = &registry.Get<InputService>();
    tfSys = &registry.Get<TransformSystem>();
}

Handle FreeCamSystem::Create(EntityID owner, Handle transform, float moveSpeed, float sens)
{
	Handle handle = CreateComp(owner);
	auto& freeCam = *Get(handle);
    freeCam       = {};
    freeCam.transform   = transform;
    freeCam.moveSpeed   = moveSpeed;
    freeCam.sensitivity = sens;
    freeCam.isActive    = true;
    return handle;
}

void FreeCamSystem::SetActive(Handle handle, bool on)
{
    if (auto cam = Get(handle))
        cam->isActive = on;
}

void FreeCamSystem::SetSpeed(Handle handle, float speed)
{
    if (auto cam = Get(handle))
        cam->moveSpeed = max(0.f, speed);
}

void FreeCamSystem::SetSensitivity(Handle handle, float sens)
{
    if (auto cam = Get(handle))
        cam->sensitivity = max(0.f, sens);
}

void FreeCamSystem::Update(float dt)
{
    ForEachAliveEx([&](Handle handle, EntityID owner, FreeCamData& cam)
        {
            if (!cam.isActive) return;
            if (!tfSys->Validate(cam.transform)) return;
           
            float dx = 0.f, dy = 0.f, dz = 0.f;
            if (input->KeyPressing(KEY::D)) dx += 1.f;
            if (input->KeyPressing(KEY::A)) dx -= 1.f;
            if (input->KeyPressing(KEY::E)) dy += 1.f;
            if (input->KeyPressing(KEY::Q)) dy -= 1.f;
            if (input->KeyPressing(KEY::W)) dz += 1.f;
            if (input->KeyPressing(KEY::S)) dz -= 1.f;

            // 대각선 가속 방지
            const float lenSq = dx * dx + dy * dy + dz * dz;
            if (lenSq > 1e-12f)
            {
                const float invLen = 1.f / sqrtf(lenSq);
                dx *= invLen; dy  *= invLen; dz *= invLen;

                const float   scale   = cam.moveSpeed * dt;
                const _float3 dtLocal = { dx * scale, dy * scale, dz * scale };
                tfSys->AddLocalOffset(cam.transform, dtLocal);
            }

            // 마우스 회전
            if (input->KeyPressing(KEY::RBUTTON))
            {
                const _float2 mouseDt = input->GetMouseDelta();
                cam.yawDeg   = WrapAngleDeg(cam.yawDeg + mouseDt.x * cam.sensitivity);
                cam.pitchDeg = clamp(cam.pitchDeg + (-mouseDt.y * cam.sensitivity), -89.f, 89.f);

                const float yawRad   = XMConvertToRadians(cam.yawDeg);
                const float pitchRad = XMConvertToRadians(cam.pitchDeg);
                tfSys->SetRotation(cam.transform, yawRad, pitchRad);
            }
        });
}

void FreeCamSystem::RenderGui(EntityID id)
{
#ifdef USE_IMGUI
    ForEachOwned(id, [&](Handle handle, FreeCamData& freeCam) 
        {
            ImGui::PushID((int)handle.idx);
            if (ImGui::CollapsingHeader("FreeCam"))
            {
                bool active = freeCam.isActive;
                if (ImGui::Checkbox("Active", &active))
                    SetActive(handle, active);

                float speed = freeCam.moveSpeed;
                if (ImGui::DragFloat("Speed", &speed, 0.1f, 0.0f, 10000.0f, "%.3f"))
                    SetSpeed(handle, max(0.0f, speed));

                float sens = freeCam.sensitivity;
                if (ImGui::DragFloat("Sensitivity", &sens, 0.001f, 0.0f, 10.0f, "%.4f"))
                    SetSensitivity(handle, max(0.0f, sens));

                ImGui::Separator();
                if (ImGui::SmallButton("Zero Rotation"))
                    tfSys->SetEuler(freeCam.transform, 0.f, 0.f, 0.f);
                ImGui::SameLine();
                if (ImGui::SmallButton("Reset Speed"))
                    SetSpeed(handle, 200.f);
            }
            ImGui::PopID();
        });
#endif
}
