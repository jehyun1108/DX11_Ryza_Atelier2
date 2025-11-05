#include "Enginepch.h"

Handle CameraSystem::Create(EntityID owner, Handle transform, float fovY, float aspect, float nearZ, float farZ)
{
    Handle handle = CreateComp(owner);
    auto& cam     = *Get(handle);
    cam           = {};
    cam.owner     = owner;
    cam.transform = transform;
    cam.fovY      = fovY;
    cam.aspect    = aspect;
    cam.nearZ     = nearZ; 
    cam.farZ      = farZ;
    XMStoreFloat4x4(&cam.view, XMMatrixIdentity());
    XMStoreFloat4x4(&cam.proj, XMMatrixIdentity());
    XMStoreFloat4x4(&cam.viewProj, XMMatrixIdentity());
    XMStoreFloat4x4(&cam.invView, XMMatrixIdentity());
    XMStoreFloat4x4(&cam.invViewProj, XMMatrixIdentity());
    XMStoreFloat4(&cam.camPos, XMVectorZero());

    if (!Validate(mainCam))
    {
        mainCam = handle;
        cam.isMainCam = true;
    }
    return handle;
}

void CameraSystem::Update(float dt)
{
    auto& tfSys = registry.Get<TransformSystem>();
    ForEachAliveEx([&](Handle handle, EntityID owner, CameraData& cam)
        {
            UpdateFollowing(cam, dt);
            RebuildMatrices(cam);
        });
}

void CameraSystem::SetPerspective(Handle handle, float fovY, float aspect, float nearZ, float farZ)
{
    if (auto cam = Get(handle)) 
    {
        cam->fovY   = fovY; 
        cam->aspect = aspect; 
        cam->nearZ  = nearZ;
        cam->farZ   = farZ; 
    }
}

void CameraSystem::SetTarget(Handle handle, Handle targetTf, _fvec offset)
{
    if (auto cam = Get(handle))
    {
        cam->targetTf = targetTf;
        XMStoreFloat3(&cam->followOffset, offset);
    }
}

void CameraSystem::ClearTarget(Handle handle)
{
    if (auto cam = Get(handle))
        cam->targetTf = {};
}

void CameraSystem::SetMainCam(Handle handle, bool isMainCam)
{
    if (!Validate(handle)) return;

    if (isMainCam) 
    {
        if (Validate(mainCam))
        {
            if (auto prevCam = Get(mainCam)) 
                prevCam->isMainCam = false;
        }
        mainCam = handle;
        if (auto curCam = Get(handle))
            curCam->isMainCam = true;
    }
    else 
    {
        if (Validate(handle))
        {
            if (auto curCam = Get(handle)) 
                curCam->isMainCam = false;
        }
        if (mainCam.idx == handle.idx && mainCam.generation == handle.generation)
            mainCam = {}; 
    }
}

void CameraSystem::SetRayPolicy(Handle handle, RAYORIGIN policy)
{
    if (auto cam = Get(handle))
        cam->rayPolicy = policy;
}

void CameraSystem::SetFollowOffsetSpace(Handle handle, OffsetSpace space)
{
    if (auto cam = Get(handle))
        cam->offsetSpace = space;
}

void CameraSystem::SetFollowPolicy(Handle handle, FollowPolicy policy, float softDamping)
{
    if (auto cam = Get(handle))
    {
        cam->followPolicy = policy;
        cam->softDamping  = softDamping;
        cam->desiredInit  = false;
    }
}

void CameraSystem::CreateRayFromScreen(Handle handle, const _float2& screenPos, const D3D11_VIEWPORT& vp, _vec& outRayOrigin, _vec& outRayDir) const
{
    if (!Validate(handle))
    {
        outRayOrigin = XMVectorZero(); 
        outRayDir = XMVectorSet(0, 0, 1, 0);
        return; 
    }
    const CameraData& cam = RequiredCam(this, handle, "CreateRayFromScreen: invalid camera");

    float ndcX, ndcY;
    ScreenToNdc(screenPos, vp, ndcX, ndcY);

    const _mat invVP = XMLoadFloat4x4(&cam.invViewProj);

    const _vec nearNdc = XMVectorSet(ndcX, ndcY, 0.f, 1.f);
    const _vec farNdc  = XMVectorSet(ndcX, ndcY, 1.f, 1.f);

    const _vec nearWorld = XMVector3TransformCoord(nearNdc, invVP);
    const _vec farWorld  = XMVector3TransformCoord(farNdc, invVP);

    const _vec dir = XMVector3Normalize(farWorld - nearWorld);

    if (cam.rayPolicy == RAYORIGIN::CameraPos)
    {
        outRayOrigin = XMLoadFloat4(&cam.camPos);
        outRayDir = dir;
    }
    else
    {
        outRayOrigin = nearWorld;
        outRayDir = dir;
    }
}

const CameraData& CameraSystem::RequiredCam(const CameraSystem* self, Handle handle, const char* what)
{
    assert(self && "CameraSystem is null");
    assert(self->Validate(handle) && what);
    const CameraData* cam = self->Get(handle);
    assert(cam && what);
    return *cam;
}

void CameraSystem::UpdateFollowing(CameraData& cam, float dt) const
{
    if (!cam.targetTf.IsValid()) return;
    auto& tfSys        = registry.Get<TransformSystem>();

    const TransformData* targetTf = tfSys.Get(cam.targetTf);
    TransformData* selfTf         = tfSys.Get(cam.transform);
    if (!targetTf || !selfTf) return;

    const _vec targetPos = XMLoadFloat3(&targetTf->pos);

    // 1. Offset 좌표계 선택
    _vec bestPos{};
    if (cam.offsetSpace == OffsetSpace::TargetSpace)
    {
        const _vec right = tfSys.GetRight(cam.targetTf);
        const _vec up    = tfSys.GetUp(cam.targetTf);
        const _vec look  = tfSys.GetLook(cam.targetTf);
        bestPos = targetPos + right * cam.followOffset.x + up * cam.followOffset.y + look * cam.followOffset.z;
    }
    else
        bestPos = targetPos + XMLoadFloat3(&cam.followOffset);

    XMStoreFloat3(&selfTf->pos, bestPos);

    // 2. 회전 Policy
    const _vec worldUp = Utility::Up();
    if (cam.followPolicy == FollowPolicy::PosOnly)
    {
        selfTf->dirty = true;
        return;
    }

    // Hard/Soft LookAt 의 목표 회전 (타깃의 약간위)
    const _vec lookAt = targetPos + XMVectorSet(0.f, 2.f, 0.f, 0.f);
    const _vec forward = XMVector3Normalize(lookAt - bestPos);
    const _vec desiredQ = Utility::BuildLookRot(forward, worldUp);

    if (cam.followPolicy == FollowPolicy::HardLookAt)
    {
        XMStoreFloat4(&selfTf->rot, desiredQ);
        selfTf->dirty = true;
        return;
    }

    // SoftLookAt
    _vec curQ = XMLoadFloat4(&selfTf->rot);
    curQ = XMQuaternionNormalize(curQ);

    if (!cam.desiredInit)
    {
        XMStoreFloat4(&selfTf->rot, desiredQ);
        cam.desiredRot = selfTf->rot;
        cam.desiredInit = true;
        selfTf->dirty = true;
        return;
    }
    const float lambda = max(0.01f, cam.softDamping); 
    const float t      = 1.f - expf(-lambda * dt);        
    const _vec outQ    = XMQuaternionSlerp(curQ, desiredQ, t);
    XMStoreFloat4(&selfTf->rot, XMQuaternionNormalize(outQ));
    selfTf->dirty = true;
}

void CameraSystem::RebuildMatrices(CameraData& cam) const
{
    auto& tfSys = registry.Get<TransformSystem>();
    const TransformData* tf = tfSys.Get(cam.transform);
    if (!tf) return;

    // 1. pos/rot 만 사용 (scale 무시)
    _vec quat = XMLoadFloat4(&tf->rot);
    quat = XMQuaternionNormalize(quat);

    _mat rotMat = XMMatrixRotationQuaternion(quat);
    _mat transMat = XMMatrixTranslation(tf->pos.x, tf->pos.y, tf->pos.z);

    _mat worldCam = rotMat * transMat;

    // 2. view / invView
    _mat view = XMMatrixInverse(nullptr, worldCam);
    XMStoreFloat4x4(&cam.view, view);
    XMStoreFloat4x4(&cam.invView, worldCam);

    // 3. Proj
    const _mat proj = XMMatrixPerspectiveFovLH(cam.fovY, cam.aspect, cam.nearZ, cam.farZ);
    XMStoreFloat4x4(&cam.proj, proj);

    // 4. viewProj / invViewProj
    const _mat viewProj = view * proj;
    XMStoreFloat4x4(&cam.viewProj, viewProj);

    const _mat invViewProj = XMMatrixInverse(nullptr, viewProj);
    XMStoreFloat4x4(&cam.invViewProj, invViewProj);

    XMStoreFloat4(&cam.camPos, _vec{ tf->pos.x, tf->pos.y, tf->pos.z, 1.f });
}

void CameraSystem::ScreenToNdc(const _float2& screenPos, const D3D11_VIEWPORT& vp, float& ndcX, float& ndcY)
{
    if (vp.Width <= 0.f || vp.Height <= 0.f) 
    {
        ndcX = 0.f;
        ndcY = 0.f;
        return;
    }
    ndcX = ((screenPos.x - vp.TopLeftX) / vp.Width) * 2.f - 1.f;
    ndcY = 1.f - ((screenPos.y - vp.TopLeftY) / vp.Height) * 2.f;
}

void CameraSystem::ExtractCameraProxy(Handle cam, CameraProxy& out) const
{
    out.view    = GetView(cam);
    out.proj    = GetProj(cam);
    out.invView = GetInvView(cam);
    XMStoreFloat4(&out.camPos, GetPos(cam));
}

void CameraSystem::RenderGui(EntityID id)
{
#ifdef USE_IMGUI
    ForEachOwned(id, [&](Handle handle, CameraData& cam)
        {
            ImGui::PushID((int)handle.idx);

            // 여러 카메라를 가질수 있으니
            if (ImGui::CollapsingHeader("Camera"))
            {
                bool isMainCam = (Validate(mainCam) && mainCam == handle);
                if (ImGui::Checkbox("MainCam", &isMainCam))
                    SetMainCam(handle, isMainCam);

                // RayPolicy
                static const char* rayPolicyTypes[] = { "CameraPos", "NearPlane" };
                int rayIdx = (cam.rayPolicy == RAYORIGIN::CameraPos) ? 0 : 1;
                if (ImGui::Combo("Ray Origin", &rayIdx, rayPolicyTypes, IM_ARRAYSIZE(rayPolicyTypes)))
                    SetRayPolicy(handle, rayIdx == 0 ? RAYORIGIN::CameraPos : RAYORIGIN::NearPlane);

                // Proj
                float fovDeg = XMConvertToDegrees(cam.fovY);
                float aspect = cam.aspect;
                float nearZ = cam.nearZ;
                float farZ = cam.farZ;

                bool projChanged = false;
                projChanged |= ImGui::DragFloat("FOV", &fovDeg, 0.1f, 1.0f, 179.0f);
                projChanged |= ImGui::DragFloat("Aspect", &aspect, 0.001f, 0.1f, 10.0f);
                projChanged |= ImGui::DragFloat("NearZ", &nearZ, 0.001f, 1e-4f, farZ - 1e-4f);
                projChanged |= ImGui::DragFloat("FarZ", &farZ, 0.1f, nearZ + 1e-4f, 1e6f);

                if (projChanged)
                    SetPerspective(handle, XMConvertToRadians(fovDeg), max(0.0001f, aspect), max(1e-4f, nearZ), max(nearZ + 1e-4f, farZ));

                // 3rd Cam
                {
                    ImGui::SeparatorText("3rd Cam");

                    ImGui::Text("TargetTf: idx=%u gen=%u %s", cam.targetTf.idx, cam.targetTf.generation, cam.targetTf.IsValid() ? "" : "(none)");

                    float offset[3] = { cam.followOffset.x, cam.followOffset.y, cam.followOffset.z };
                    if (ImGui::DragFloat3("Follow Offset", offset, 0.01f))
                        cam.followOffset = _float3(offset[0], offset[1], offset[2]);
                    if (cam.targetTf.IsValid())
                    {
                        if (ImGui::SmallButton("Clear Target"))
                            ClearTarget(handle);
                    }
                    else
                    {
                        ImGui::BeginDisabled();
                        ImGui::SmallButton("Clear Target");
                        ImGui::EndDisabled();
                    }
                }
            }
            ImGui::PopID();
        });
#endif
}