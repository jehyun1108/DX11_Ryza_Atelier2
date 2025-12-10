#include "Enginepch.h"
#include "ScreenDistortionSystem.h"

void ScreenDistortionSystem::OnBoot()
{
	camSys = &registry.Get<CameraSystem>();
}

void ScreenDistortionSystem::Tick(float dt)
{
    if (!state.active) return;

    auto& cb = state.cb;

    cb.time += dt;
    float t = cb.time / cb.duration;

    if (t >= 1.0f)
    {
        state.active = false;
        cb.time = 0.0f;
        cb.radius = 0.0f;
        cb.strength = 0.0f;
        cb.duration = 0.0f;
        return;
    }

    const float maxRadius = 0.8f;
    cb.radius = maxRadius * t;
}

void ScreenDistortionSystem::StartBattleToField(const _float3& centerWorld)
{
    const _float4x4& viewProjMat = camSys->GetMainViewProj();
    _mat viewProj = XMLoadFloat4x4(&viewProjMat);

    _vec pW   = XMVectorSet(centerWorld.x, centerWorld.y, centerWorld.z, 1.0f);
    _vec clip = XMVector4Transform(pW, viewProj);
    _vec ndc  = XMVectorDivide(clip, XMVectorSplatW(clip));

    _float4 ndc4;
    XMStoreFloat4(&ndc4, ndc);

    DistortionCB cb{};

    cb.centerUV.x  =  ndc4.x * 0.5f + 0.5f;
    cb.centerUV.y  = -ndc4.y * 0.5f + 0.5f;
    cb.radius      = 0.0f;
    cb.thickness   = 0.06f;
    cb.strength    = 0.05f;
    cb.time        = 0.0f;
    cb.duration    = 0.8f;
    cb.paddingDist = 0.0f;

    state.cb = cb;
    state.active = true;
}