#include "Enginepch.h"

void CameraSystem::OnBoot()
{
    tfSys = &registry.Get<TransformSystem>();
}

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
    XMStoreFloat4x4(&cam.view,        XMMatrixIdentity());
    XMStoreFloat4x4(&cam.proj,        XMMatrixIdentity());
    XMStoreFloat4x4(&cam.viewProj,    XMMatrixIdentity());
    XMStoreFloat4x4(&cam.invView,     XMMatrixIdentity());
    XMStoreFloat4x4(&cam.invViewProj, XMMatrixIdentity());
    XMStoreFloat4(&cam.camPos,        XMVectorZero());

    if (!Validate(mainCam))
    {
        mainCam = handle;
        cam.isMainCam = true;
    }
    return handle;
}

void CameraSystem::Update(float dt)
{
    ForEachAliveEx([&](Handle handle, EntityID owner, CameraData& cam)
        {
            UpdateFollowing(cam, dt);
            RebuildMatrices(cam);
        });
}

void CameraSystem::SetPerspective(Handle handle, float fovY, float aspect, float nearZ, float farZ)
{
    auto cam = Get(handle);
    cam->projType = ProjectionType::Perspective;
    cam->fovY     = fovY; 
    cam->aspect   = aspect; 
    cam->nearZ    = nearZ;
    cam->farZ     = farZ; 
}

void CameraSystem::SetOrtho(Handle handle, float width, float height, float nearZ, float farZ)
{
    auto cam = Get(handle);
    cam->projType    = ProjectionType::Orthographic;
    cam->orthoWidth  = width;
    cam->orthoHeight = height;
    cam->nearZ       = nearZ;
    cam->farZ        = farZ;
}

void CameraSystem::SetTarget(Handle handle, Handle targetTf, _fvec offset)
{
    auto cam = Get(handle);
    cam->targetTf = targetTf;
    XMStoreFloat3(&cam->followOffset, offset);
}

void CameraSystem::ClearTarget(Handle handle)
{
    auto cam = Get(handle);
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
    auto cam = Get(handle);
    cam->rayPolicy = policy;
}

void CameraSystem::SetFollowOffsetSpace(Handle handle, OffsetSpace space)
{
    auto cam = Get(handle);
    cam->offsetSpace = space;
}

void CameraSystem::SetFollowPolicy(Handle handle, FollowPolicy policy, float softDamping)
{
    auto cam = Get(handle);
    cam->followPolicy = policy;
    cam->softDamping  = softDamping;
    cam->desiredInit  = false;
}

void CameraSystem::SetLookAtOffset(Handle handle, _fvec offset)
{
    auto cam = Get(handle);
    XMStoreFloat3(&cam->lookAtOffset, offset);
}

void CameraSystem::CreateRayFromScreen(Handle h, const _float2& sp, const D3D11_VIEWPORT& vp, _vec& outOri, _vec& outDir) const
{
    const CameraData& cam = RequiredCam(this, h, "CreateRayFromScreen");

    float sx   = (sp.x - vp.TopLeftX) / vp.Width;
    float sy   = (sp.y - vp.TopLeftY) / vp.Height;
    float ndcX = sx * 2.f - 1.f;
    float ndcY = 1.f - sy * 2.f;

    _vec pNearNdc = XMVectorSet(ndcX, ndcY, 0.0f, 1.0f);
    _vec pFarNdc  = XMVectorSet(ndcX, ndcY, 1.0f, 1.0f);

    _mat invVP   = XMLoadFloat4x4(&cam.invViewProj);
    _mat invView = XMLoadFloat4x4(&cam.invView);

    _vec pNearWorld = XMVector3TransformCoord(pNearNdc, invVP);
    _vec pFarWorld  = XMVector3TransformCoord(pFarNdc, invVP);

    _vec dir = XMVector3Normalize(pFarWorld - pNearWorld);

    if (cam.rayPolicy == RAYORIGIN::NearPlane)
        outOri = pNearWorld;
    else 
        outOri = XMLoadFloat4(&cam.camPos);

    outDir = dir;

    _vec fwdWorld = XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0, 0, 1, 0), invView));
    float cosF = XMVectorGetX(XMVector3Dot(fwdWorld, dir));
}

const CameraData& CameraSystem::RequiredCam(const CameraSystem* self, Handle handle, const char* what)
{
    const CameraData* cam = self->Get(handle);
    return *cam;
}

void CameraSystem::UpdateFollowing(CameraData& cam, float dt) const
{
    if (!cam.targetTf.IsValid()) return;
    const TransformData* targetTf = tfSys->Get(cam.targetTf);
    TransformData* selfTf         = tfSys->Get(cam.transform);

    const _vec targetPos = XMLoadFloat3(&targetTf->pos);
    _vec bestPos{};
    if (cam.offsetSpace == OffsetSpace::TargetSpace)
    {
        const _vec right = tfSys->GetRight(cam.targetTf);
        const _vec up    = tfSys->GetUp(cam.targetTf);
        const _vec look  = tfSys->GetLook(cam.targetTf);
        bestPos = targetPos + right * cam.followOffset.x + up * cam.followOffset.y + look * cam.followOffset.z;
    }
    else
        bestPos = targetPos + XMLoadFloat3(&cam.followOffset);

    XMStoreFloat3(&selfTf->pos, bestPos);
    const _vec worldUp = Utility::Up();
    if (cam.followPolicy == FollowPolicy::PosOnly)
    {
        selfTf->dirty = true;
        return;
    }

    const _vec lookAt = targetPos + XMLoadFloat3(&cam.lookAtOffset);
    const _vec forward = XMVector3Normalize(lookAt - bestPos);
    const _vec desiredQ = Utility::BuildLookRot(forward, worldUp);

    if (cam.followPolicy == FollowPolicy::HardLookAt)
    {
        XMStoreFloat4(&selfTf->rot, desiredQ);
        selfTf->dirty = true;
        return;
    }
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
    const TransformData* tf = tfSys->Get(cam.transform);
    _mat worldCam = XMLoadFloat4x4(&tf->world);

    // 2. view / invView
    _mat view = XMMatrixInverse(nullptr, worldCam);
    XMStoreFloat4x4(&cam.view, view);
    XMStoreFloat4x4(&cam.invView, worldCam);

    // 3. Proj / invProj
    _mat proj{};
    if (cam.projType == ProjectionType::Perspective)
        proj = XMMatrixPerspectiveFovLH(cam.fovY, cam.aspect, cam.nearZ, cam.farZ);
    else
        proj = XMMatrixOrthographicLH(cam.orthoWidth, cam.orthoHeight, cam.nearZ, cam.farZ);

    XMStoreFloat4x4(&cam.proj, proj);
    const _mat invProj = XMMatrixInverse(nullptr, proj);
    XMStoreFloat4x4(&cam.invProj, invProj);

    // 4. viewProj, invViewProj
    const _mat viewProj = view * proj;
    const _mat invViewProj = XMMatrixInverse(nullptr, viewProj);
    XMStoreFloat4x4(&cam.viewProj, viewProj);
    XMStoreFloat4x4(&cam.invViewProj, invViewProj);
    XMStoreFloat4(&cam.camPos, { tf->world._41, tf->world._42, tf->world._43, 1.0f });
    _vec forward = XMVector3Normalize( XMVector3TransformNormal(XMVectorSet(0.f, 0.f, 1.f, 0.f), worldCam));
    XMStoreFloat4(&cam.forward, forward);
}

void CameraSystem::ScreenToNdc(const _float2& screenPos, const D3D11_VIEWPORT& vp, float& ndcX, float& ndcY)
{
    const float sx = (screenPos.x - vp.TopLeftX) / vp.Width;
    const float sy = (screenPos.y - vp.TopLeftY) / vp.Height;
    ndcX = sx * 2.f - 1.f;
    ndcY = 1.f - sy * 2.f;
}

void CameraSystem::ExtractCameraProxy(Handle cam, CameraProxy& out) const
{
    out.view        = GetView(cam);
    out.proj        = GetProj(cam);
    out.viewProj    = GetViewProj(cam);
    out.invView     = GetInvView(cam);
    out.invProj     = GetInvProj(cam);
    out.invViewProj = GetInvViewProj(cam);
    out.zNear       = GetNearZ(cam);
    out.zFar        = GetFarZ(cam);
    out.fovY        = GetFovY(cam);
    out.aspect      = GetAspect(cam);
    
    XMStoreFloat4(&out.camPos, GetPos(cam));
    const CameraData* src = Get(cam);
    out.camForward = src->forward;
}

void CameraSystem::RenderGui(EntityID id)
{
#ifdef USE_IMGUI
    ForEachOwned(id, [&](Handle handle, CameraData& cam)
        {
            ImGui::PushID((int)handle.idx);

            if (ImGui::CollapsingHeader("Camera"))
            {
                bool isMainCam = (Validate(mainCam) && mainCam == handle);
                if (ImGui::Checkbox("MainCam", &isMainCam))
                    SetMainCam(handle, isMainCam);

                static const char* rayPolicyTypes[] = { "CameraPos", "NearPlane" };
                int rayIdx = (cam.rayPolicy == RAYORIGIN::CameraPos) ? 0 : 1;
                if (ImGui::Combo("Ray Origin", &rayIdx, rayPolicyTypes, IM_ARRAYSIZE(rayPolicyTypes)))
                    SetRayPolicy(handle, rayIdx == 0 ? RAYORIGIN::CameraPos : RAYORIGIN::NearPlane);

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