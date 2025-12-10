#include "Enginepch.h"
#include "NavMeshSystem.h"

static inline _float3 Sub(const _float3& a, const _float3& b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
static inline _float3 Add(const _float3& a, const _float3& b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
static inline _float3 Mul(const _float3& a, float s) { return { a.x * s, a.y * s, a.z * s }; }
static inline float    Dot(const _float3& a, const _float3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static inline _float3  Cross(const _float3& a, const _float3& b) { return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x }; }
static inline float    LenSq(const _float3& a) { return Dot(a, a); }
static inline float    Len(const _float3& a) { return sqrt(LenSq(a)); }
static inline _float3  NormalizeSafe(const _float3& a) { float l = Len(a); return (l > 1e-8f) ? Mul(a, 1.0f / l) : _float3{ 0, 1, 0 }; }
static inline uint64_t MakeEdgeKey(_uint a, _uint b) { if (a > b) swap(a, b); return (uint64_t(a) << 32) | uint64_t(b); }
static float DistPointSegSq(const _float3& p, const _float3& a, const _float3& b)
{
    const _float3 ab = Sub(b, a);
    const _float3 ap = Sub(p, a);
    const float ab2 = LenSq(ab);
    if (ab2 <= 0.f) return LenSq(Sub(p, a));
    float t = Dot(ap, ab) / ab2;
    if (t < 0.f) t = 0.f;
    else if (t > 1.f) t = 1.f;
    const _float3 q = Add(a, Mul(ab, t));
    return LenSq(Sub(p, q));
}
static bool PointInTriangle(const _float3& p,const _float3& a, const _float3& b, const _float3& c)
{
    const _float3 v0 = Sub(b, a);
    const _float3 v1 = Sub(c, a);
    const _float3 v2 = Sub(p, a);

    const float d00 = Dot(v0, v0);
    const float d01 = Dot(v0, v1);
    const float d11 = Dot(v1, v1);
    const float d20 = Dot(v2, v0);
    const float d21 = Dot(v2, v1);

    const float denom = d00 * d11 - d01 * d01;
    if (fabsf(denom) < 1e-6f)
        return false;

    const float v = (d11 * d20 - d01 * d21) / denom;
    const float w = (d00 * d21 - d01 * d20) / denom;
    const float u = 1.0f - v - w;

    const float eps = -1e-3f; 
    return (u >= eps && v >= eps && w >= eps);
}
static _float3 ClosestPointOnSeg(const _float3& p, const _float3& a, const _float3& b)
{
    const _float3 ab = Sub(b, a);
    const float   ab2 = LenSq(ab);
    if (ab2 <= 0.f) return a; 

    float t = Dot(Sub(p, a), ab) / ab2;
    if (t < 0.f) t = 0.f;
    else if (t > 1.f) t = 1.f;

    return Add(a, Mul(ab, t));
}
// ========================================================================================================================================
void NavMeshSystem::ClearAll()
{
    data.store.vertices.clear();
    data.store.triangles.clear();
    data.temp = NavTempBuffer{};
}

bool NavMeshSystem::PushPointFromPick(const _float3& hitPos, const _float3& hitNormal)
{
    _uint v = FindSnapVertex(hitPos, snapRadius);
    bool snappedVertex = (v != UINT32_MAX);

    _float3 finalPos = hitPos;

    if (snappedVertex)
        finalPos = data.store.vertices[v].posWorld;
    else
    {
        _float3 edgePoint{};
        const bool snappedEdge = FindNearestPointEdge(hitPos, snapRadius, edgePoint);

        if (snappedEdge)
            finalPos = edgePoint;

        v = AppendVertex(finalPos);
    }

    const _uint k = data.temp.count;
    data.temp.p[k] = finalPos;
    data.temp.snappedVid[k] = snappedVertex ? v : UINT32_MAX;
    data.temp.snappedExisting[k] = snappedVertex;
    ++data.temp.count;

    if (data.temp.count == 3)
    {
        const _uint a = FindSnapVertex(data.temp.p[0], 0.f) != UINT32_MAX ? FindSnapVertex(data.temp.p[0], 0.f) : AppendVertex(data.temp.p[0]);
        const _uint b = FindSnapVertex(data.temp.p[1], 0.f) != UINT32_MAX ? FindSnapVertex(data.temp.p[1], 0.f) : AppendVertex(data.temp.p[1]);
        const _uint c = FindSnapVertex(data.temp.p[2], 0.f) != UINT32_MAX ? FindSnapVertex(data.temp.p[2], 0.f) : AppendVertex(data.temp.p[2]);

        const _float3 pa = data.store.vertices[a].posWorld;
        const _float3 pb = data.store.vertices[b].posWorld;
        const _float3 pc = data.store.vertices[c].posWorld;

        const _float3 n = NormalizeSafe(Cross(Sub(pb, pa), Sub(pc, pa)));
        const float s = Dot(n, NormalizeSafe(hitNormal));
        if (s < 0.f) data.store.triangles.push_back({ a, c, b });
        else         data.store.triangles.push_back({ a, b, c });

        data.temp = NavTempBuffer{}; 
        return true;
    }
    return false;
}

void NavMeshSystem::MoveVertex(_uint vid, const _float3& newPos)
{
    assert(vid < data.store.vertices.size());
    data.store.vertices[vid].posWorld = newPos;
}

bool NavMeshSystem::DeleteLastTriangle()
{
    const auto n = data.store.triangles.size();
    if (n == 0) return false;
    data.store.triangles.pop_back();
    data.temp = NavTempBuffer{}; 
    return true;
}

bool NavMeshSystem::UndoLastPoint()
{
    if (data.temp.count == 0) return false;
    --data.temp.count;
    return true;
}

bool NavMeshSystem::RaycastDown(const _float3& origin, float maxDist, _float3& outHitPos, _float3& outNormal) const
{
    const auto& vs = data.store.vertices;
    const auto& ts = data.store.triangles;
    if (vs.empty() || ts.empty())
        return false;

    const _float3 dir{ 0.f, -1.f, 0.f };

    bool   hit = false;
    float  bestT = maxDist;
    _float3 bestPos{};
    _float3 bestN{ 0.f, 1.f, 0.f };

    for (const auto& t : ts)
    {
        const _float3 a = vs[t.a].posWorld;
        const _float3 b = vs[t.b].posWorld;
        const _float3 c = vs[t.c].posWorld;

        const _float3 ab = Sub(b, a);
        const _float3 ac = Sub(c, a);
        const _float3 n = Cross(ab, ac); 

        const float ndotDir = Dot(n, dir);
        if (fabsf(ndotDir) < 1e-6f)
            continue; 

        const float tParam = Dot(Sub(a, origin), n) / ndotDir;
        if (tParam < 0.f || tParam > bestT)
            continue;

        const _float3 p = Add(origin, Mul(dir, tParam));

        if (!PointInTriangle(p, a, b, c))
            continue;

        const _float3 nNorm = NormalizeSafe(n);
        if (nNorm.y <= 0.f)
            continue;

        hit = true;
        bestT = tParam;
        bestPos = p;
        bestN = nNorm;
    }

    if (!hit)
        return false;

    outHitPos = bestPos;
    outNormal = bestN;
    return true;
}

bool NavMeshSystem::SampleHeight(const _float3& pos, _float3& outPos, _float3& outNormal) const
{
    const _float3& origin = { pos.x, pos.y + 100.f, pos.z };
    const float maxDist = 1000.f;

    return RaycastDown(origin, maxDist, outPos, outNormal);
}

_uint NavMeshSystem::AppendVertex(const _float3& p)
{
    const _uint id = static_cast<_uint>(data.store.vertices.size());
    data.store.vertices.push_back({ p });
    return id;
}

void NavMeshSystem::FindNearestEdge(const _float3& p, _uint& outI, _uint& outJ) const
{
    assert(!data.store.triangles.empty());
    const auto& vs = data.store.vertices;
    const auto& ts = data.store.triangles;

    float bestD2 = FLT_MAX;
    _uint bi = UINT32_MAX, bj = UINT32_MAX;

    for (const auto& t : ts)
    {
        const _float3 a = vs[t.a].posWorld;
        const _float3 b = vs[t.b].posWorld;
        const _float3 c = vs[t.c].posWorld;

        float d2ab = DistPointSegSq(p, a, b);
        if (d2ab < bestD2) { bestD2 = d2ab; bi = t.a; bj = t.b; }

        float d2bc = DistPointSegSq(p, b, c);
        if (d2bc < bestD2) { bestD2 = d2bc; bi = t.b; bj = t.c; }

        float d2ca = DistPointSegSq(p, c, a);
        if (d2ca < bestD2) { bestD2 = d2ca; bi = t.c; bj = t.a; }
    }

    outI = bi; outJ = bj;
}

void NavMeshSystem::MakeTriangleWithWinding(_uint k, _uint i, _uint j, const _float3& hitNormals)
{
    assert(k < data.store.vertices.size() && i < data.store.vertices.size() && j < data.store.vertices.size());
    const _float3 pk = data.store.vertices[k].posWorld;
    const _float3 pi = data.store.vertices[i].posWorld;
    const _float3 pj = data.store.vertices[j].posWorld;

    const _float3 n = NormalizeSafe(Cross(Sub(pi, pk), Sub(pj, pk)));
    const float s = Dot(n, NormalizeSafe(hitNormals));
    if (s < 0.f) data.store.triangles.push_back({ k, j, i });
    else         data.store.triangles.push_back({ k, i, j });
}

_uint NavMeshSystem::FindSnapVertex(const _float3& p, float radius) const
{
    const auto& vs = data.store.vertices;
    if (vs.empty()) return UINT32_MAX;

    const float r2 = radius * radius;
    float bestD2 = FLT_MAX;
    _uint best = UINT32_MAX;

    for (_uint i = 0; i < vs.size(); ++i)
    {
        const float d2 = LenSq(Sub(vs[i].posWorld, p));
        if (d2 <= r2 && d2 < bestD2)
        {
            bestD2 = d2;
            best = i;
        }
    }
    return best;
}

void NavMeshSystem::BuildDebugLines(vector<VertexColor>& out) const
{
    out.clear();
    const auto& vs = data.store.vertices;
    const auto& ts = data.store.triangles;

    const _float4 colTri = { 1, 1, 0, 1 };
    for (const auto& t : ts)
    {
        const _float3 a = vs[t.a].posWorld;
        const _float3 b = vs[t.b].posWorld;
        const _float3 c = vs[t.c].posWorld;
        out.push_back({ a, colTri }); out.push_back({ b, colTri });
        out.push_back({ b, colTri }); out.push_back({ c, colTri });
        out.push_back({ c, colTri }); out.push_back({ a, colTri });
    }

    const _float4 colTemp = { 0, 1, 1, 1 };
    for (_uint i = 0; i < data.temp.count; ++i)
    {
        const _float3 p = data.temp.p[i];
        const float s = 0.05f;
        out.push_back({ { p.x - s, p.y, p.z }, colTemp }); out.push_back({ { p.x + s, p.y, p.z }, colTemp });
        out.push_back({ { p.x, p.y, p.z - s }, colTemp }); out.push_back({ { p.x, p.y, p.z + s }, colTemp });

        if (i > 0)
        {
            const _float3 q = data.temp.p[i - 1];
            out.push_back({ q, colTemp }); out.push_back({ p, colTemp });
        }
    }

    const _float4 colSnap = { 0.2f, 0.9f, 0.2f, 1 };
    for (_uint i = 0; i < data.temp.count; ++i)
    {
        if (data.temp.snappedExisting[i])
            AppendCircle(out, data.temp.p[i], 0.15f, 24, colSnap);
    }
}

void NavMeshSystem::BuildDebugTriangles(vector<VertexColor>& out) const
{
    out.clear();
    const auto& vs = data.store.vertices;
    const auto& ts = data.store.triangles;

    const _float4 colTri = { 0.f, 1.f, 0.f, 0.3f }; 

    for (const auto& t : ts)
    {
        const _float3 a = vs[t.a].posWorld;
        const _float3 b = vs[t.b].posWorld;
        const _float3 c = vs[t.c].posWorld;

        out.push_back({ a, colTri });
        out.push_back({ b, colTri });
        out.push_back({ c, colTri });
    }
}

void NavMeshSystem::AppendCircle(vector<VertexColor>& out, const _float3& c, float r, _uint seg, const _float4& col) const
{
    if (seg < 8) seg = 8;
    float prevx = c.x + r, prevz = c.z;
    for (_uint i = 1; i <= seg; ++i)
    {
        const float t = (2.f * 3.1415926535f * i) / float(seg);
        const float x = c.x + r * cosf(t);
        const float z = c.z + r * sinf(t);
        out.push_back({ { prevx, c.y, prevz }, col });
        out.push_back({ { x, c.y, z }, col });
        prevx = x; prevz = z;
    }
}

bool NavMeshSystem::FindNearestPointEdge(const _float3& p, float radius, _float3& outPoint) const
{
    const auto& vs = data.store.vertices;
    const auto& ts = data.store.triangles;
    if (vs.empty() || ts.empty())
        return false;

    const float r2 = radius * radius;

    bool   found = false;
    float  bestD2 = r2;
    _float3 bestP{};

    for (const auto& t : ts)
    {
        const _float3 a = vs[t.a].posWorld;
        const _float3 b = vs[t.b].posWorld;
        const _float3 c = vs[t.c].posWorld;

        // edge AB
        {
            _float3 q = ClosestPointOnSeg(p, a, b);
            float d2 = LenSq(Sub(p, q));
            if (d2 < bestD2)
            {
                bestD2 = d2;
                bestP = q;
                found = true;
            }
        }

        // edge BC
        {
            _float3 q = ClosestPointOnSeg(p, b, c);
            float d2 = LenSq(Sub(p, q));
            if (d2 < bestD2)
            {
                bestD2 = d2;
                bestP = q;
                found = true;
            }
        }

        // edge CA
        {
            _float3 q = ClosestPointOnSeg(p, c, a);
            float d2 = LenSq(Sub(p, q));
            if (d2 < bestD2)
            {
                bestD2 = d2;
                bestP = q;
                found = true;
            }
        }
    }

    if (!found)
        return false;

    outPoint = bestP;
    return true;
}

bool NavMeshSystem::Save(filesystem::path& path)
{
    ofstream ofs(path, std::ios::binary);
    const _uint vc = (_uint)data.store.vertices.size();
    const _uint tc = (_uint)data.store.triangles.size();
    ofs.write((const char*)&vc, sizeof(vc));
    ofs.write((const char*)&tc, sizeof(tc));
    if (vc) ofs.write((const char*)data.store.vertices.data(), vc * sizeof(NavVertex));
    if (tc) ofs.write((const char*)data.store.triangles.data(), tc * sizeof(NavTriangle));
    return (bool)ofs;
}

bool NavMeshSystem::Load(filesystem::path& path)
{
    ifstream ifs(path, std::ios::binary);
    _uint vc = 0, tc = 0;
    ifs.read((char*)&vc, sizeof(vc));
    ifs.read((char*)&tc, sizeof(tc));
    data.store.vertices.resize(vc);
    data.store.triangles.resize(tc);
    if (vc) ifs.read((char*)data.store.vertices.data(), vc * sizeof(NavVertex));
    if (tc) ifs.read((char*)data.store.triangles.data(), tc * sizeof(NavTriangle));
    data.temp = NavTempBuffer{};
    return (bool)ifs;
}