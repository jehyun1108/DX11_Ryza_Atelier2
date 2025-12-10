#include "Enginepch.h"

void TrailSystem::OnBoot()
{
	particleSys = &registry.Get<ParticleSystem>();
}

Handle TrailSystem::Create(EntityID owner, const TrailDesc& desc)
{
	Handle handle = CreateComp(owner);
	auto& data    = *Get(handle);
	data.points.clear();
	data.elapsed  = 0.f;
	data.desc     = desc;
	data.owner    = owner;
	return handle;
}

void TrailSystem::AddSample(Handle trailHandle, const _float3& tipPos)
{
	TrailInstance* trail = Get(trailHandle);

	auto& desc = trail->desc;
	auto& pts = trail->points;

	TrailPoint pt{};
	pt.pos = tipPos;
	pt.age = 0.f;

	if (pts.empty())
	{
		pts.push_back(pt);
		return;
	}

	const _float3& last = pts.back().pos;

	float dx = tipPos.x - last.x;
	float dy = tipPos.y - last.y;
	float dz = tipPos.z - last.z;

	float distSq = dx * dx + dy * dy + dz * dz;
	float minD = desc.minSegDist;
	float minDSq = minD * minD;

	if (distSq >= minDSq)
		pts.push_back(pt);
}

void TrailSystem::Tick(float dt)
{
	ForEachAliveEx([&](Handle h, EntityID owner, TrailInstance& t)
		{
			auto& pts = t.points;
			if (!pts.empty())
			{
				for (auto& pt : pts)
					pt.age += dt;

				const float life = t.desc.lifeTime;
				size_t firstAlive = 0;
				while (firstAlive < pts.size() && pts[firstAlive].age > life)
					++firstAlive;

				if (firstAlive > 0)
					pts.erase(pts.begin(), pts.begin() + firstAlive);
			}

			TrailDesc& desc = t.desc;
			if (!desc.sparkEnabled) return;
			if (pts.size() < 2)     return;

			t.sparkAccum += dt;

			while (t.sparkAccum >= desc.sparkInterval)
			{
				t.sparkAccum -= desc.sparkInterval;

				for (int i = 0; i < desc.sparkBurstCount; ++i)
				{
					float u = Utility::Range(0.f, 1.f);
					_float3 basePos = SamplePathPos(t, u);

					particleSys->SpawnBurst(desc.spark, basePos, 1, 0u);
				}
			}
		});
}

void TrailSystem::ExtractTrailSnapshot(TrailSnapshot& out, CameraProxy& cam)
{
	out.Clear();

	ForEachAliveEx([&](Handle h, EntityID owner, TrailInstance& t)
		{
			auto& pts = t.points;
			if (pts.size() < 2) return;

			const TrailDesc& desc = t.desc;
			const float life = desc.lifeTime;

			TrailDrawItem item{};
			item.desc = &desc;
			item.points.reserve(pts.size());

			for (auto& p : pts)
			{
				float tNorm = Utility::Saturate(p.age / life);

				TrailPointProxy proxy{};
				proxy.pos = p.pos;
				proxy.t = tNorm;

				item.points.push_back(proxy);
			}

			const TrailPoint& last = pts.back();
			_mat view = XMLoadFloat4x4(&cam.view);
			_vec pw = XMLoadFloat3(&last.pos);
			_vec pv = XMVector3TransformCoord(pw, view);
			item.camDist = fabsf(XMVectorGetZ(pv));
			out.trails.push_back(std::move(item));
		});
}

_float TrailSystem::ComputePathLength(const TrailInstance& t) const
{
	const auto& pts = t.points;
	size_t n = pts.size();
	if (n < 2) return 0.f;

	float total = 0.f;
	for (size_t i = 1; i < n; ++i)
	{
		const _float3& a = pts[i - 1].pos;
		const _float3& b = pts[i].pos;

		float dx = b.x - a.x;
		float dy = b.y - a.y;
		float dz = b.z - a.z;

		total += sqrtf(dx * dx + dy * dy + dz * dz);
	}
	return total;
}

_float3 TrailSystem::SamplePathPos(const TrailInstance& t, float u) const
{
	const auto& pts = t.points;
	size_t n = pts.size();
	assert(n >= 1);

	if (n == 1)
		return pts[0].pos;

	float total = ComputePathLength(t);
	if (total <= 0.f)
		return pts.back().pos;

	float target = total * Utility::Saturate(u);

	float accum = 0.f;
	for (size_t i = 1; i < n; ++i)
	{
		const _float3& a = pts[i - 1].pos;
		const _float3& b = pts[i].pos;

		float dx = b.x - a.x;
		float dy = b.y - a.y;
		float dz = b.z - a.z;
		float len = sqrtf(dx * dx + dy * dy + dz * dz);

		if (target <= accum + len)
		{
			float tSeg = (target - accum) / len;

			_float3 p{};
			p.x = a.x + (b.x - a.x) * tSeg;
			p.y = a.y + (b.y - a.y) * tSeg;
			p.z = a.z + (b.z - a.z) * tSeg;
			return p;
		}
		accum += len;
	}

	return pts.back().pos;
}
