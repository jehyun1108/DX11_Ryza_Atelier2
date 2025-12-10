#include "pch.h"
#include "CamSpawner.h"

constexpr float aspect = static_cast<float>(WinX) / static_cast<float>(WinY);

CamSpawner::CamSpawner(SystemRegistry& registry) : registry(registry)
{
	spawner = &registry.Get<EntitySpawner>();
	camSys = &registry.Get<CameraSystem>();
}

EntityHandles CamSpawner::SpawnFreeCam(const _float3& pos, float fovYDeg, float nearZ, float farZ, float moveSpeed, bool makeMain)
{
    auto cam = spawner->NewEntity()
        .WithTf(TransformDesc{ .pos = pos })
        .WithLayer(LayerUtil::LayerBit(LAYER::CAMERA))
        .WithCam(XMConvertToRadians(fovYDeg), aspect, nearZ, farZ, makeMain)
        .WithFreeCam(moveSpeed)
        .WithTag("freecam")
        .Build();

    return cam;
}

EntityHandles CamSpawner::SpawnFieldOrbitCam(Handle targetTf, float fovDeg, float initYaw, float initPitch, float initDist, float nearZ, float farZ, bool makeMain)
{
    auto cam = spawner->NewEntity()
        .WithTf()
        .WithLayer(LayerUtil::LayerBit(LAYER::CAMERA))
        .WithCam(XMConvertToRadians(fovDeg), aspect, nearZ, farZ, makeMain)
        .WithOrbitCam(targetTf, initYaw, initPitch, initDist)
        .WithTag("orbitcam")
        .Build();

    return cam;
}

EntityHandles CamSpawner::SpawnFieldMiniCam()
{
    auto cam = spawner->NewEntity()
        .WithTf()
        .WithLayer(LayerUtil::LayerBit(LAYER::CAMERA))
        .WithCam(XMConvertToRadians(60.f), aspect, 1.f, 3000.f, false)
        .Build();

    return cam;
}
