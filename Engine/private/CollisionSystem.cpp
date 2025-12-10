#include "Enginepch.h"
// ================================================================================================
static bool IntersectRaySphere(const _float3& ro, const _float3& rd, const _float3& c, float r, float& outT, _float3& outN)
{
    _float3 oc{ ro.x - c.x, ro.y - c.y, ro.z - c.z };
    float b = oc.x * rd.x + oc.y * rd.y + oc.z * rd.z;
    float c2 = oc.x * oc.x + oc.y * oc.y + oc.z * oc.z - r * r;
    float disc = b * b - c2;
    if (disc < 0.f) return false;
    float t = -b - sqrtf(disc);
    if (t < 0.f) t = -b + sqrtf(disc);
    if (t < 0.f) return false;
    outT = t;
    _float3 p{ ro.x + rd.x * t, ro.y + rd.y * t, ro.z + rd.z * t };
    _float3 n{ (p.x - c.x) / r, (p.y - c.y) / r, (p.z - c.z) / r };
    outN = n;
    return true;
}

static bool IntersectRayAABB(const _float3& ro, const _float3& rd, const BoundingBox& b, float& outT, _float3& outN)
{
    _float3 mn = b.Center; mn.x -= b.Extents.x; mn.y -= b.Extents.y; mn.z -= b.Extents.z;
    _float3 mx = b.Center; mx.x += b.Extents.x; mx.y += b.Extents.y; mx.z += b.Extents.z;

    float tmin = 0.f, tmax = FLT_MAX; _float3 n{ 0, 0, 0 };

    auto axis = [&](float roA, float rdA, float mnA, float mxA, _float3 faceN)->bool
        {
            if (fabsf(rdA) < 1e-8f) { if (roA < mnA || roA > mxA) return false; return true; }
            float ood = 1.f / rdA;
            float t1 = (mnA - roA) * ood;
            float t2 = (mxA - roA) * ood;
            _float3 n1 = faceN, n2{ -faceN.x, -faceN.y, -faceN.z };
            if (t1 > t2) { std::swap(t1, t2); std::swap(n1, n2); }
            if (t1 > tmin) { tmin = t1; n = n1; }
            if (t2 < tmax) { tmax = t2; }
            return tmin <= tmax;
        };

    if (!axis(ro.x, rd.x, mn.x, mx.x, { -1, 0, 0 })) return false;
    if (!axis(ro.y, rd.y, mn.y, mx.y, { 0, -1, 0 })) return false;
    if (!axis(ro.z, rd.z, mn.z, mx.z, { 0, 0, -1 })) return false;

    if (tmin < 0.f) { outT = 0.f; outN = n; return true; } 
    outT = tmin; outN = n; return true;
}

static bool IntersectRayOBB(const _float3& ro, const _float3& rd, const BoundingOrientedBox& o, float& outT, _float3& outN)
{
    _float3 C = o.Center, E = o.Extents;
    auto q = XMLoadFloat4(&o.Orientation);
    auto R = XMMatrixRotationQuaternion(q);
    auto Rt = XMMatrixTranspose(R);

    auto roW = XMLoadFloat3(&ro);
    auto rdW = XMLoadFloat3(&rd);
    auto cW = XMLoadFloat3(&C);

    auto roL = XMVector3Transform(roW - cW, Rt);
    auto rdL = XMVector3TransformNormal(rdW, Rt);

    _float3 roL3, rdL3; XMStoreFloat3(&roL3, roL); XMStoreFloat3(&rdL3, rdL);

    BoundingBox aabbL({ 0, 0, 0 }, E);
    float t; _float3 nL;
    if (!IntersectRayAABB(roL3, rdL3, aabbL, t, nL)) return false;

    outT = t;
    auto nW4 = XMVector3TransformNormal(XMLoadFloat3(&nL), R);
    XMStoreFloat3(&outN, XMVector3Normalize(nW4));
    return true;
}
static bool IntersectRayTri(const _float3& ro, const _float3& rd, const _float3& a, const _float3& b, const _float3& c, float& t, _float3& n)
{
    const _float3 ab{ b.x - a.x, b.y - a.y, b.z - a.z };
    const _float3 ac{ c.x - a.x, c.y - a.y, c.z - a.z };
    _float3 p{ rd.y * ac.z - rd.z * ac.y,
        rd.z * ac.x - rd.x * ac.z,
        rd.x * ac.y - rd.y * ac.x };
    float det = ab.x * p.x + ab.y * p.y + ab.z * p.z;
    if (fabsf(det) < 1e-8f) return false;
    float inv = 1.0f / det;

    _float3 s{ ro.x - a.x, ro.y - a.y, ro.z - a.z };
    float u = (s.x * p.x + s.y * p.y + s.z * p.z) * inv;
    if (u < 0.f || u > 1.f) return false;

    _float3 q{ s.y * ab.z - s.z * ab.y,
        s.z * ab.x - s.x * ab.z,
        s.x * ab.y - s.y * ab.x };
    float v = (rd.x * q.x + rd.y * q.y + rd.z * q.z) * inv;
    if (v < 0.f || u + v > 1.f) return false;

    float tt = (ac.x * q.x + ac.y * q.y + ac.z * q.z) * inv;
    if (tt < 0.f) return false;

    t = tt;

    _float3 nn{
        ab.y * ac.z - ab.z * ac.y,
        ab.z * ac.x - ab.x * ac.z,
        ab.x * ac.y - ab.y * ac.x
    };
    float len = sqrtf(nn.x * nn.x + nn.y * nn.y + nn.z * nn.z);
    if (len > 1e-8f) { nn.x /= len; nn.y /= len; nn.z /= len; }
    n = nn;
    return true;
}
inline _float3 ReadPos(const Mesh* m, uint32_t vi)
{
    const uint8_t* base = m->GetCPUVertexBytes();
    const uint32_t stride = m->GetVertexStride();
    const _float3* p = reinterpret_cast<const _float3*>(base + stride * vi);
    return *p; 
}

inline uint32_t ReadIdx(const Mesh* m, uint32_t ii)
{
    const uint8_t* ib = m->GetCPUIndexBytes();
    if (m->GetIdxFormat() == DXGI_FORMAT_R16_UINT)
        return reinterpret_cast<const uint16_t*>(ib)[ii];
    else
        return reinterpret_cast<const uint32_t*>(ib)[ii];
}

inline _float3 TransformPoint(const _float3& v, const _float4x4& wm)
{
    _vec p = XMVectorSet(v.x, v.y, v.z, 1.f);
    _vec q = XMVector3TransformCoord(p, XMLoadFloat4x4(&wm));
    _float3 o; XMStoreFloat3(&o, q);
    return o;
}
// A,B가 충돌 대상?
inline bool CanCollide(Mask aBelongs, Mask aMask, Mask bBelongs, Mask bMask)
{
    return (aMask & bBelongs) && (bMask & aBelongs);
}
// Raycast, Overlap 검사에서 target이 후보인가 ? 
inline bool MatchesQuery(Mask queryMask, Mask targetBelongs)
{
    return (queryMask & targetBelongs) != 0;
}
// ------------------------------- Create ---------------------------------------------------------
void CollisionSystem::OnBoot()
{
    tfSys    = &registry.Get<TransformSystem>();
    modelSys = &registry.Get<ModelSystem>();
}

Handle CollisionSystem::CreateAABB(EntityID owner, Handle tfHandle, const BoundingBox& localBox)
{
	Handle handle = CreateComp(owner);
    auto data = Get(handle);
	data->type          = ColliderType::AABB;
	data->enabled       = true;
	data->tf            = tfHandle;
	data->aabb.local    = localBox;
	data->aabb.world    = localBox;
    data->belongsTo     = Bit(CollisionLayer::Prop);
    data->collidesWith  = Bit(CollisionLayer::Ground) | Bit(CollisionLayer::Prop);
	return handle;
}

Handle CollisionSystem::CreateSphere(EntityID owner, Handle tfHandle, const _float3& centerLocal, float radiusLocal)
{
	Handle handle            = CreateComp(owner);
    auto data                = Get(handle);
	data->type               = ColliderType::Sphere;
	data->enabled            = true;
	data->tf                 = tfHandle;
	data->sphere.centerLocal = centerLocal;
	data->sphere.radiusLocal = radiusLocal;
	data->sphere.centerWorld = centerLocal;
	data->sphere.radiusWorld = radiusLocal;
    data->belongsTo          = Bit(CollisionLayer::Prop);
    data->collidesWith       = Bit(CollisionLayer::Ground) | Bit(CollisionLayer::Prop);
	return handle;
}

Handle CollisionSystem::CreateOBB(EntityID owner, Handle tfHandle, const BoundingOrientedBox& localOBB)
{
	Handle handle      = CreateComp(owner);
    auto data          = Get(handle);
	data->type         = ColliderType::OBB;
	data->enabled      = true;
	data->tf           = tfHandle;
	data->obb.local    = localOBB;
	data->obb.world    = localOBB;
    data->belongsTo    = Bit(CollisionLayer::Prop);
    data->collidesWith = Bit(CollisionLayer::Ground) | Bit(CollisionLayer::Prop);				  
	return handle;
}

Handle CollisionSystem::CreateMeshRay(EntityID owner, Handle tfHandle, const Mesh* mesh, Mask belongs, Mask collides)
{
    Handle handle       = CreateComp(owner);
    auto& data          = *Get(handle);
    data.type           = ColliderType::MeshRay;
    data.enabled        = true;
    data.tf             = tfHandle;
    data.mesh.mesh      = mesh;
    data.mesh.tf        = tfHandle;
    data.belongsTo      = belongs;
    data.collidesWith   = collides;
    data.mesh.localAABB = mesh->GetLocalAABB();
    data.mesh.worldAABB = data.mesh.localAABB;
    data.mesh.worldMat  = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
    return handle;
}

// ----------------------- Runtime switching --------------------------------------
void CollisionSystem::SetAABB(Handle handle, const BoundingBox& localBox)
{
    auto data = Get(handle);
	data->type = ColliderType::AABB;
	data->aabb.local = localBox;
}

void CollisionSystem::SetSphere(Handle handle, const _float3& centerLocal, float radiusLocal)
{
    auto data = Get(handle);
	data->type               = ColliderType::Sphere;
	data->sphere.centerLocal = centerLocal;
	data->sphere.radiusLocal = radiusLocal;
}

void CollisionSystem::SetOBB(Handle handle, const BoundingOrientedBox& localOBB)
{
    auto data = Get(handle);
	data->type = ColliderType::OBB;
	data->obb.local = localOBB;
}

// ----------------------------- Utility --------------------------------------------
void CollisionSystem::SetEnabled(Handle handle, bool enabled)
{
    auto data = Get(handle);
	data->enabled = enabled;
}

void CollisionSystem::SetTransform(Handle handle, Handle tfHandle)
{
    auto data = Get(handle);
	data->tf = tfHandle;
}

void CollisionSystem::SetBelongsTo(Handle handle, _uint layer)
{
    auto data = Get(handle);
    data->belongsTo = layer;
}

void CollisionSystem::SetCollidesWith(Handle handle, _uint mask)
{
    auto data = Get(handle);
    data->collidesWith = mask;
}

void CollisionSystem::Tick(float dt)
{
	ForEachAliveEx([&](Handle handle, EntityID owner, CollisionData& data)
		{
			if (!data.enabled) return;

			const TransformData* tf  = tfSys->Get(data.tf);
            const _float4x4&     mat = tf->world;
            _mat worldMat = XMLoadFloat4x4(&mat);

            switch (data.type)
            {
            case ColliderType::AABB:
                data.aabb.local.Transform(data.aabb.world, worldMat);
                break;

            case ColliderType::Sphere:
            {
                _vec centerLeft = XMLoadFloat3(&data.sphere.centerLocal);
                _vec centerWorld = XMVector3TransformCoord(centerLeft, worldMat);
                XMStoreFloat3(&data.sphere.centerWorld, centerWorld);

                _float3 centerX{ mat._11, mat._21, mat._31 };
                _float3 centerY{ mat._12, mat._22, mat._32 };
                _float3 centerZ{ mat._13, mat._23, mat._33 };
                float scaleX = Utility::Length(centerX);
                float scaleY = Utility::Length(centerY);
                float scaleZ = Utility::Length(centerZ);
                float maxScale = max(scaleX, max(scaleY, scaleZ));
                data.sphere.radiusWorld = data.sphere.radiusLocal * maxScale;
            }
            break;

            case ColliderType::OBB:
                data.obb.local.Transform(data.obb.world, worldMat);
                break;

            case ColliderType::MeshRay:
                data.mesh.localAABB.Transform(data.mesh.worldAABB, worldMat);
                data.mesh.worldMat = mat;
                break;
            }
		});
}

void CollisionSystem::ExtractColliderProxies(vector<ColliderProxy>& out) const
{
    const_cast<CollisionSystem*>(this)->ForEachAliveEx([&](Handle handle, EntityID owner, CollisionData& data)
        {
            if (!data.enabled) return;

            ColliderProxy proxy{};
            switch (data.type)
            {
            case ColliderType::AABB:
                proxy.type = ColliderType::AABB;
                proxy.aabb = data.aabb.world;
                break;

            case ColliderType::OBB:
                proxy.type = ColliderType::OBB;
                proxy.obb = data.obb.world;
                break;

            case ColliderType::Sphere:
                proxy.type = ColliderType::Sphere;
                proxy.sphereCenter = data.sphere.centerWorld;
                proxy.sphereRadius = data.sphere.radiusWorld;
                break;
            }
            out.emplace_back(proxy);
        });
}

RayHit CollisionSystem::RayCast(const RayDesc& ray) const
{
    RayHit best{};

    const_cast<CollisionSystem*>(this)->ForEachAliveEx([&](Handle handle, EntityID owner, CollisionData& data) 
        {
            if (!data.enabled) return;
            if (!MatchesQuery(ray.queryMask, data.belongsTo)) return;

            float t; _float3 normal; bool ok = false;
            switch (data.type)
            {
            case ColliderType::Sphere:
                ok = IntersectRaySphere(ray.origin, ray.dir, data.sphere.centerWorld, data.sphere.radiusWorld, t, normal);
                break;
                
            case ColliderType::AABB:
                ok = IntersectRayAABB(ray.origin, ray.dir, data.aabb.world, t, normal);
                break;

            case ColliderType::OBB:
                ok = IntersectRayOBB(ray.origin, ray.dir, data.obb.world, t, normal);
                break;

            case ColliderType::MeshRay:
            {
                float tbox; _float3 nbox;
                if (!IntersectRayAABB(ray.origin, ray.dir, data.mesh.worldAABB, tbox, nbox))
                    return;

                const Mesh* m = data.mesh.mesh;
                const _uint triCount = m->GetIdxCount() / 3;

                for (_uint i = 0; i < triCount; ++i)
                {
                    const _uint i0 = ReadIdx(m, i * 3 + 0);
                    const _uint i1 = ReadIdx(m, i * 3 + 1);
                    const _uint i2 = ReadIdx(m, i * 3 + 2);

                    _float3 aL = ReadPos(m, i0);
                    _float3 bL = ReadPos(m, i1);
                    _float3 cL = ReadPos(m, i2);

                    _float3 a = TransformPoint(aL, data.mesh.worldMat);
                    _float3 b = TransformPoint(bL, data.mesh.worldMat);
                    _float3 c = TransformPoint(cL, data.mesh.worldMat);

                    float tt; _float3 nn;
                    if (!IntersectRayTri(ray.origin, ray.dir, a, b, c, tt, nn)) continue;
                    if (tt < 0.f || tt > ray.maxDist) continue;

                    if (!best.hit || tt < best.t)
                    {
                        best.hit = true;
                        best.t = tt;
                        best.normal = nn;
                        best.pos = { ray.origin.x + ray.dir.x * tt,
                            ray.origin.y + ray.dir.y * tt,
                            ray.origin.z + ray.dir.z * tt };
                        best.handle = handle;
                        best.owner = owner;
                    }
                }
                return; 
            }
            }

            if (!ok) return;
            if (t < 0.f || t > ray.maxDist) return;

            if (!best.hit || t < best.t)
            {
                best.hit = true;
                best.t = t;
                best.normal = normal;
                best.pos = { ray.origin.x + ray.dir.x * t,
                    ray.origin.y + ray.dir.y * t,
                    ray.origin.z + ray.dir.z * t };
                best.handle = handle;
                best.owner = owner;
            }
        });
    return best;
}


bool CollisionSystem::GetObbTipPoint(EntityID owner, _float3& outTip) const
{
    const CollisionData* bestData = nullptr;
    float    bestExtent = 0.f;
    int      bestAxis = 0;

    const_cast<CollisionSystem*>(this)->ForEachAliveEx(
        [&](Handle h, EntityID e, CollisionData& data)
        {
            if (e != owner) return;
            if (!data.enabled) return;
            if (data.type != ColliderType::OBB) return;

            const auto& obb = data.obb.world;
            const _float3& ext = obb.Extents;

            float axisExt[3] = { ext.x, ext.y, ext.z };
            for (int i = 0; i < 3; ++i)
            {
                if (axisExt[i] > bestExtent)
                {
                    bestExtent = axisExt[i];
                    bestAxis = i;
                    bestData = &data;
                }
            }
        });

    if (!bestData)
        return false;

    const auto& obb = bestData->obb.world;
    const _float3& c = obb.Center;
    const _float3& ext = obb.Extents;

    _vec q = XMLoadFloat4(&obb.Orientation);
    _mat R = XMMatrixRotationQuaternion(q);

    _float3 axisLocal;
    if (bestAxis == 0)      axisLocal = _float3{ 1.f, 0.f, 0.f };
    else if (bestAxis == 1) axisLocal = _float3{ 0.f, 1.f, 0.f };
    else                    axisLocal = _float3{ 0.f, 0.f, 1.f };

    _vec aL = XMLoadFloat3(&axisLocal);
    _vec aW = XMVector3Normalize(XMVector3TransformNormal(aL, R));
    _float3 axis;
    XMStoreFloat3(&axis, aW);

    float len = (bestAxis == 0) ? ext.x : (bestAxis == 1 ? ext.y : ext.z);

    _float3 endA{
        c.x + axis.x * len,
        c.y + axis.y * len,
        c.z + axis.z * len
    };
    _float3 endB{
        c.x - axis.x * len,
        c.y - axis.y * len,
        c.z - axis.z * len
    };

    Handle dummy{};
    TransformData* rootTf = tfSys->GetByOwner(owner, &dummy);
    _float3 rootPos = rootTf ? rootTf->pos : c;

    auto Dist2 = [&rootPos](_float3 p)
        {
            float dx = p.x - rootPos.x;
            float dy = p.y - rootPos.y;
            float dz = p.z - rootPos.z;
            return dx * dx + dy * dy + dz * dz;
        };

    outTip = (Dist2(endA) > Dist2(endB)) ? endA : endB;
    return true;
}

bool CollisionSystem::FindWeaponHit(EntityID attacker, BattleHitInfo& outHit) const
{
    const CollisionData* weapon = nullptr;
    vector<pair<const CollisionData*, EntityID>> monsters;

    const_cast<CollisionSystem*>(this)->ForEachAliveEx( [&](Handle handle, EntityID owner, CollisionData& data)
        {
            if (!data.enabled) return;
            if (data.type != ColliderType::OBB) return;

            if (Has(data.belongsTo, CollisionLayer::Trigger))
            {
                    weapon = &data;
            }
            else if (Has(data.belongsTo, CollisionLayer::Character))
            {
                if (owner != attacker)
                    monsters.emplace_back(&data, owner);
            }
        });

    if (!weapon)
        return false;

    const BoundingOrientedBox& wob = weapon->obb.world;

    for (auto& pair : monsters)
    {
        const CollisionData* mData = pair.first;
        EntityID             mOwner = pair.second;

        if (!CanCollide(weapon->belongsTo, weapon->collidesWith,
            mData->belongsTo, mData->collidesWith))
            continue;

        if (!wob.Intersects(mData->obb.world))
            continue;

        outHit.attacker = attacker;
        outHit.target = mOwner;
        outHit.centerWorld = mData->obb.world.Center; // 부딪힌 몬스터 위치
        return true;
    }

    return false;
}

void CollisionSystem::RenderGui(EntityID id)
{
#ifdef USE_IMGUI
    ForEachOwned(id, [&](Handle handle, CollisionData& data)
        {
            ImGui::PushID((int)handle.idx);

            if (ImGui::CollapsingHeader("Collision"))
            {

                GuiUtility::BeginPanel("Collision", PanelMode::Lines, 12);
                {
                    ImGui::TextUnformatted("Collider");
                    ImGui::SameLine();

                    const char* typeText = (data.type == ColliderType::AABB) ? "AABB" : (data.type == ColliderType::Sphere) ? "Sphere" : "OBB";
                    GuiUtility::Badge(typeText, ImVec4(0.20f, 0.55f, 1.00f, 1.0f));

                    ImGui::SameLine();
                    bool enabled = data.enabled;
                    if (ImGui::Checkbox("Enabled", &enabled))
                        SetEnabled(handle, enabled);

                }

                ImGui::Separator();

                // Change Types
                {
                    auto TypeButton = [&](const char* label, ColliderType targetType, const ImVec4& onColor, const ImVec4& offColor)
                        {
                            const bool isCurrent = (data.type == targetType);
                            ImGui::PushStyleColor(ImGuiCol_Button, isCurrent ? onColor : offColor);
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, isCurrent ? onColor :
                                ImVec4(offColor.x * 1.1f, offColor.y * 1.1f, offColor.z * 1.1f, offColor.w));
                            ImGui::PushStyleColor(ImGuiCol_ButtonActive, isCurrent ? onColor : offColor);
                            const bool clicked = ImGui::Button(label);
                            ImGui::PopStyleColor(3);
                            return clicked;
                        };

                    const ImVec4 onColor = ImVec4(0.24f, 0.72f, 0.38f, 1.0f);
                    const ImVec4 offColor = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);

                    ImGui::TextUnformatted("Type");
                    ImGui::SameLine();

                    // → AABB
                    if (TypeButton("AABB", ColliderType::AABB, onColor, offColor))
                    {
                        BoundingBox aabbLocal{};
                        if (data.type == ColliderType::Sphere)
                            aabbLocal = Utility::ToAABBFromSphere(data.sphere.centerLocal, data.sphere.radiusLocal);
                        else if (data.type == ColliderType::OBB)
                            aabbLocal = Utility::ToAABBFromOBB(data.obb.local);
                        else
                            aabbLocal = data.aabb.local;

                        SetAABB(handle, aabbLocal);
                    }

                    ImGui::SameLine();

                    // → Sphere
                    if (TypeButton("Sphere", ColliderType::Sphere, onColor, offColor))
                    {
                        _float3 centerLocal{};
                        float   radiusLocal = 0.5f;

                        if (data.type == ColliderType::AABB)
                        {
                            auto sphere = Utility::ToSphereFromAABB(data.aabb.local);
                            centerLocal = sphere.first;
                            radiusLocal = sphere.second;
                        }
                        else if (data.type == ColliderType::OBB)
                        {
                            BoundingBox aabbLocal = Utility::ToAABBFromOBB(data.obb.local);
                            auto sphere = Utility::ToSphereFromAABB(aabbLocal);
                            centerLocal = sphere.first;
                            radiusLocal = sphere.second;
                        }
                        else
                        {
                            centerLocal = data.sphere.centerLocal;
                            radiusLocal = data.sphere.radiusLocal;
                        }

                        SetSphere(handle, centerLocal, max(radiusLocal, 1e-6f));
                    }

                    ImGui::SameLine();

                    // → OBB
                    if (TypeButton("OBB", ColliderType::OBB, onColor, offColor))
                    {
                        BoundingOrientedBox obbLocal{};
                        if (data.type == ColliderType::AABB)
                            obbLocal = Utility::ToOBBFromAABB(data.aabb.local);

                        else if (data.type == ColliderType::Sphere)
                        {
                            BoundingBox aabbLocal = Utility::ToAABBFromSphere(data.sphere.centerLocal, data.sphere.radiusLocal);
                            obbLocal = Utility::ToOBBFromAABB(aabbLocal);
                        }
                        else
                            obbLocal = data.obb.local;

                        SetOBB(handle, obbLocal);
                    }
                }

                ImGui::Separator();

                if (data.type == ColliderType::AABB)
                {
                    _float3 centerLocal = data.aabb.local.Center;
                    _float3 extentsLocal = data.aabb.local.Extents;

                    ImGui::TextDisabled("AABB (Local)");
                    ImGui::DragFloat3("Center", &centerLocal.x, 0.01f);
                    if (ImGui::DragFloat3("Extents", &extentsLocal.x, 0.01f, 1e-6f, 1e6f))
                    {
                        extentsLocal.x = max(extentsLocal.x, 1e-6f);
                        extentsLocal.y = max(extentsLocal.y, 1e-6f);
                        extentsLocal.z = max(extentsLocal.z, 1e-6f);
                    }

                    if (ImGui::Button("Apply##AABB"))
                    {
                        BoundingBox aabbLocal(centerLocal, extentsLocal);
                        SetAABB(handle, aabbLocal);
                    }
                }
                else if (data.type == ColliderType::Sphere)
                {
                    _float3 centerLocal = data.sphere.centerLocal;
                    float   radiusLocal = data.sphere.radiusLocal;

                    ImGui::TextDisabled("Sphere (Local)");
                    ImGui::DragFloat3("Center", &centerLocal.x, 0.01f);
                    ImGui::DragFloat("Radius", &radiusLocal, 0.01f, 1e-6f, 1e6f);

                    if (ImGui::Button("Apply##Sphere"))
                        SetSphere(handle, centerLocal, max(radiusLocal, 1e-6f));

                }
                else
                {
                    _float3 centerLocal     = data.obb.local.Center;
                    _float3 extentsLocal    = data.obb.local.Extents;
                    _float4 orientationQuat = data.obb.local.Orientation; 

                    ImGui::TextDisabled("OBB (Local)");
                    ImGui::DragFloat3("Center", &centerLocal.x, 0.01f);
                    if (ImGui::DragFloat3("Extents", &extentsLocal.x, 0.01f, 1e-6f, 1e6f))
                    {
                        extentsLocal.x = max(extentsLocal.x, 1e-6f);
                        extentsLocal.y = max(extentsLocal.y, 1e-6f);
                        extentsLocal.z = max(extentsLocal.z, 1e-6f);
                    }

                    if (ImGui::DragFloat4("Orientation (x,y,z,w)", &orientationQuat.x, 0.01f, -1.f, 1.f))
                    {
                        XMVECTOR quat = XMVector4Normalize(XMLoadFloat4(&orientationQuat));
                        XMStoreFloat4(&orientationQuat, quat);
                    }

                    if (ImGui::Button("Apply##OBB"))
                    {
                        BoundingOrientedBox obbLocal{};
                        obbLocal.Center = centerLocal;
                        obbLocal.Extents = extentsLocal;
                        obbLocal.Orientation = orientationQuat;
                        SetOBB(handle, obbLocal);
                    }
                }
                GuiUtility::EndPanel();
            }
            ImGui::PopID();
        });
#endif
}