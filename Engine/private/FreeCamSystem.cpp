#include "Enginepch.h"

namespace
{
    inline float WrapAngleDeg(float a)
    {
        // [-180, 180) ¡§±‘»≠
        a = fmodf(a, 360.f);
        if (a < -180.f)  a += 360.f;
        if (a >= 180.f)  a -= 360.f;
        return a;
    }
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

    registry.Get<TransformSystem>().SetSpeed(transform, moveSpeed);
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
    {
        cam->moveSpeed = speed;
        registry.Get<TransformSystem>().SetSpeed(cam->transform, speed);
    }
}

void FreeCamSystem::SetSensitivity(Handle handle, float sens)
{
    if (auto cam = Get(handle))
        cam->sensitivity = max(0.f, sens);
}

void FreeCamSystem::Update(float dt)
{
    auto& tfSys = registry.Get<TransformSystem>();
    GameInstance& input = GameInstance::GetInstance();

    ForEachAlive([&](_uint, FreeCamData& freeCam) 
        {
            if (!freeCam.isActive) return;
            TransformData* tf = tfSys.Get(freeCam.transform);
            if (!tf) return;

            const float speed = freeCam.moveSpeed;
            if (input.KeyPressing(KEY::W)) tfSys.Move(freeCam.transform, MOVE::FORWARD, dt);
            if (input.KeyPressing(KEY::S)) tfSys.Move(freeCam.transform, MOVE::BACK, dt);
            if (input.KeyPressing(KEY::A)) tfSys.Move(freeCam.transform, MOVE::LEFT, dt);
            if (input.KeyPressing(KEY::D)) tfSys.Move(freeCam.transform, MOVE::RIGHT, dt);
            if (input.KeyPressing(KEY::Q)) tfSys.Move(freeCam.transform, MOVE::UP, dt);
            if (input.KeyPressing(KEY::E)) tfSys.Move(freeCam.transform, MOVE::DOWN, dt);

            if (input.KeyPressing(KEY::RBUTTON))
            {
                const _float2 delta = input.GetMouseDelta();

                freeCam.yawDeg   += delta.x * freeCam.sensitivity;
                freeCam.pitchDeg += -delta.y * freeCam.sensitivity;   

                freeCam.pitchDeg = clamp(freeCam.pitchDeg, -89.0f, 89.0f);
                freeCam.yawDeg = WrapAngleDeg(freeCam.yawDeg);

                const float yawRad   = XMConvertToRadians(freeCam.yawDeg);
                const float pitchRad = XMConvertToRadians(freeCam.pitchDeg);

                tfSys.SetRotation(freeCam.transform, yawRad, pitchRad);
            }
        });
}

void FreeCamSystem::RenderGui(EntityID id)
{
#ifdef USE_IMGUI
    auto& tfSys = registry.Get<TransformSystem>();
    ForEachOwned(id, [&](Handle handle, FreeCamData& freeCam) 
        {
            ImGui::PushID((int)handle.idx);
            if (ImGui::CollapsingHeader("FreeCam"))
            {
                bool active = freeCam.isActive;
                if (ImGui::Checkbox("Active", &active))
                    SetActive(handle, active);

                float speed = freeCam.moveSpeed;
                if (ImGui::DragFloat("Spped", &speed, 0.1f, 0.0f, 10000.0f, "%.3f"))
                    SetSpeed(handle, max(0.0f, speed));

                float sens = freeCam.sensitivity;
                if (ImGui::DragFloat("Sensitivity", &sens, 0.001f, 0.0f, 10.0f, "%.4f"))
                    SetSensitivity(handle, max(0.0f, sens));

                ImGui::Separator();
                if (ImGui::SmallButton("Zero Rotation"))
                    tfSys.SetEuler(freeCam.transform, 0.f, 0.f, 0.f);
                ImGui::SameLine();
                if (ImGui::SmallButton("Reset Speed"))
                    SetSpeed(handle, 200.f);
            }
            ImGui::PopID();
        });
#endif
}
