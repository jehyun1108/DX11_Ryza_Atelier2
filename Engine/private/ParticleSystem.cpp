#include "Enginepch.h"
#include "ParticleSystem.h"

Handle ParticleSystem::CreateSpawner(EntityID owner, const ParticleSpawnData* spawnData, const _float3& worldPos)
{
	Handle handle      = CreateComp(owner);
	auto&  spawner     = *Get(handle);
	spawner.data       = spawnData;
	spawner.worldPos   = worldPos;
	spawner.elapsed    = 0.f;
	spawner.spawnAccum = 0.f;
    spawner.owner      = 0;
	return handle;
}

void ParticleSystem::OnBoot()
{
	renderSys = &registry.Get<RenderSystem>();
	particles.resize(maxParticles);
	aliveCount = 0;
}

void ParticleSystem::ExtractParticleSnapshot(ParticleSnapshot& out, const CameraProxy& cam)
{
    out.transparent.clear();
    out.transparent.reserve(static_cast<size_t>(aliveCount));

    for (int i = 0; i < aliveCount; ++i)
    {
        const Particle& particle = particles[static_cast<size_t>(i)];
        const float camDist = renderSys->CalcCamDist(particle.pos, cam);

        ParticleDrawItem item{};
        item.pos = particle.pos;
        item.size = particle.size;
        item.color = particle.color;
        item.rotRad = particle.rotRad;
        item.camDist = camDist;
        item.texKey = particle.texKey;

        const ParticleSpawnData& spawnData = *particle.data;

        if (spawnData.sheet.enabled)
        {
            const SpriteSheetInfo& sheet = spawnData.sheet;

            int cols = sheet.cols;
            int rows = sheet.rows;
            if (cols <= 0) cols = 1;
            if (rows <= 0) rows = 1;

            const int total = cols * rows;

            int frame = particle.sheetFrame;

            if (frame < 0)        frame = 0;
            if (frame >= total)   frame = total - 1;

            const int col = frame % cols;
            const int row = frame / cols;

            const float du = 1.f / static_cast<float>(cols);
            const float dv = 1.f / static_cast<float>(rows);

            item.uvMin.x = col * du;
            item.uvMin.y = row * dv;
            item.uvMax.x = (col + 1) * du;
            item.uvMax.y = (row + 1) * dv;
        }
        else
        {
            item.uvMin = _float2(0.f, 0.f);
            item.uvMax = _float2(1.f, 1.f);
        }

        out.transparent.emplace_back(item);
    }
}

void ParticleSystem::SpawnBurst(const ParticleSpawnData& spawnData, const _float3& worldPos, int count, EffectHandle owner)
{
    if (count <= 0) return;

    for (int n = 0; n < count; ++n)
    {
        if (aliveCount >= maxParticles) return;

        const float rLife = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        const float rSpeed = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        const float rSpread = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

        const float lifeTime = spawnData.lifeMin + (spawnData.lifeMax - spawnData.lifeMin) * rLife;
        const float speed = spawnData.speedMin + (spawnData.speedMax - spawnData.speedMin) * rSpeed;

        assert(lifeTime > 0.f);
        assert(speed >= 0.f);

        // 1) 위치 오프셋 먼저 계산
        _float3 offset{ 0.f, 0.f, 0.f };
        _float3 spawnPos = worldPos;

        if (spawnData.posRadiusMax > 0.f)
        {
            float r0 = spawnData.posRadiusMin;
            float r1 = spawnData.posRadiusMax;
            if (r1 < r0) r1 = r0;

            float ur = Utility::Range(0.f, 1.f);
            float radius = r0 + (r1 - r0) * ur;

            float u = Utility::Range(-1.f, 1.f);
            float theta = Utility::Range(0.f, XM_2PI);
            float s = sqrtf(1.f - u * u);

            offset.x = radius * s * cosf(theta);
            offset.y = radius * u;
            offset.z = radius * s * sinf(theta);

            spawnPos.x += offset.x;
            spawnPos.y += offset.y;
            spawnPos.z += offset.z;
        }

        // 2) 방향 계산
        _float3 dir{};

        if (spawnData.velFromPos && (offset.x != 0.f || offset.y != 0.f || offset.z != 0.f))
        {
            // 위치 오프셋 방향으로 바깥으로
            float len2 = offset.x * offset.x + offset.y * offset.y + offset.z * offset.z;
            float inv = 1.f / sqrtf(len2);

            dir.x = offset.x * inv;
            dir.y = offset.y * inv;
            dir.z = offset.z * inv;
        }
        else
        {
            // 기존 방식: baseDir + spreadAng
            _float3 baseDir = spawnData.baseDir;

            const float angle = (rSpread * 2.f - 1.f) * spawnData.spreadAng;

            _vec vDir = XMLoadFloat3(&baseDir);
            _mat rotY = XMMatrixRotationY(angle);
            vDir = XMVector3TransformNormal(vDir, rotY);
            XMStoreFloat3(&dir, vDir);
        }

        const int idx = aliveCount;
        Particle& particle = particles[static_cast<size_t>(idx)];
        ++aliveCount;

        particle.data = &spawnData;
        particle.texKey = spawnData.texKey;
        particle.aliveTime = 0.f;
        particle.lifeTime = lifeTime;
        particle.pos = spawnPos;
        particle.velocity.x = dir.x * speed;
        particle.velocity.y = dir.y * speed;
        particle.velocity.z = dir.z * speed;
        particle.size = spawnData.startSize;
        particle.color = spawnData.startColor;
        particle.owner = owner;

        if (spawnData.randomStartRot)
            particle.rotRad = Utility::Range(0.f, XM_2PI);
        else
            particle.rotRad = 0.f;

        if (spawnData.rotSpeedMin != 0.f || spawnData.rotSpeedMax != 0.f)
        {
            const float rRot = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            particle.rotSpeed =
                spawnData.rotSpeedMin + (spawnData.rotSpeedMax - spawnData.rotSpeedMin) * rRot;
        }
        else
            particle.rotSpeed = 0.f;

        particle.sheetFrame = 0;
        if (spawnData.sheet.enabled)
            particle.sheetFrame = spawnData.sheet.startFrame;
    }
}


void ParticleSystem::SetSpawnerBasis(Handle handle, const _float3& right, const _float3& up, const _float3& forward, bool useBasis)
{
	ParticleSpawnerInstance* sp = Get(handle);
	sp->basisRight   = right;
	sp->basisUp      = up;
	sp->basisForward = forward;
	sp->useBasis     = useBasis;
}

void ParticleSystem::SetSpawnerOwner(Handle handle, EffectHandle effectHandle)
{
    ParticleSpawnerInstance* sp = Get(handle);
    sp->owner = effectHandle;
}

void ParticleSystem::KillByOwner(EffectHandle effectHandle)
{
    int i = 0;
    while (i < aliveCount)
    {
        Particle& p = particles[static_cast<size_t>(i)];
        if (p.owner == effectHandle)
        {
            particles[static_cast<size_t>(i)] = particles[static_cast<size_t>(aliveCount - 1)];
            --aliveCount;
            continue;
        }
        ++i;
    }
}

void ParticleSystem::Tick(float dt)
{
    int i = 0;
    while (i < aliveCount)
    {
        Particle& particle = particles[static_cast<size_t>(i)];
        particle.aliveTime += dt;
        if (particle.aliveTime >= particle.lifeTime)
        {
            particles[static_cast<size_t>(i)] = particles[static_cast<size_t>(aliveCount - 1)];
            --aliveCount;
            continue;
        }

        particle.pos.x += particle.velocity.x * dt;
        particle.pos.y += particle.velocity.y * dt;
        particle.pos.z += particle.velocity.z * dt;
        particle.rotRad += particle.rotSpeed * dt;

        float t = Utility::Saturate(particle.aliveTime / particle.lifeTime);
        const ParticleSpawnData& spawnData = *particle.data;

        const float sizeW = EffectUtility::Curve(spawnData.sizeCurve, t);
        particle.size = spawnData.startSize + (spawnData.endSize - spawnData.startSize) * sizeW;

        const float colorW = EffectUtility::Curve(spawnData.colorCurve, t);
        particle.color.x = spawnData.startColor.x + (spawnData.endColor.x - spawnData.startColor.x) * colorW;
        particle.color.y = spawnData.startColor.y + (spawnData.endColor.y - spawnData.startColor.y) * colorW;
        particle.color.z = spawnData.startColor.z + (spawnData.endColor.z - spawnData.startColor.z) * colorW;

        const float alphaW = EffectUtility::Curve(spawnData.alphaCurve, t);
        particle.color.w = spawnData.startColor.w + (spawnData.endColor.w - spawnData.startColor.w) * alphaW;

        if (spawnData.sheet.enabled)
        {
            const SpriteSheetInfo& sheet = spawnData.sheet;

            const int first = sheet.startFrame;
            const int last = sheet.endFrame;
            const int count = (last >= first) ? (last - first + 1) : 0;

            if (count > 0)
            {
                if (!sheet.animate)
                    particle.sheetFrame = first;
                else
                {
                    int frameOffset = 0;

                    if (sheet.fps > 0.f)
                    {
                        float frameFloat = sheet.fps * particle.aliveTime;
                        int   raw = static_cast<int>(frameFloat);

                        if (sheet.loop)
                        {
                            frameOffset = raw % count;
                            if (frameOffset < 0)
                                frameOffset += count;
                        }
                        else
                        {
                            if (raw < 0)      raw = 0;
                            if (raw >= count) raw = count - 1;
                            frameOffset = raw;
                        }
                    }
                    else
                    {
                        float u = Utility::Saturate(t);
                        int raw = static_cast<int>(u * count);
                        if (raw >= count) raw = count - 1;
                        frameOffset = raw;
                    }

                    particle.sheetFrame = first + frameOffset;
                }
            }
        }
        ++i;
    }

    ForEachAliveEx([&](Handle handle, EntityID owner, ParticleSpawnerInstance& spawner)
        {
            const ParticleSpawnData& spawnData = *spawner.data;

            spawner.elapsed += dt;
            spawner.spawnAccum += spawnData.spawnRate * dt;

            int spawnCount = static_cast<int>(spawner.spawnAccum);
            if (spawnCount <= 0) return;

            spawner.spawnAccum -= static_cast<float>(spawnCount);

            float rMin = spawnData.posRadiusMin;
            float rMax = spawnData.posRadiusMax;
            if (rMin < 0.f) rMin = 0.f;
            if (rMax < rMin) rMax = rMin;

            for (int n = 0; n < spawnCount; ++n)
            {
                if (aliveCount >= maxParticles) return;

                const float rLife = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                const float rSpeed = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                const float rSpread = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

                const float lifeTime = spawnData.lifeMin + (spawnData.lifeMax - spawnData.lifeMin) * rLife;
                const float speed = spawnData.speedMin + (spawnData.speedMax - spawnData.speedMin) * rSpeed;

                // 1) 위치 오프셋 먼저
                _float3 offset{ 0.f, 0.f, 0.f };
                _float3 spawnPos = spawner.worldPos;

                if (spawnData.posRadiusMax > 0.f)
                {
                    float r0 = rMin;
                    float r1 = rMax;

                    float ur = Utility::Range(0.f, 1.f);
                    float radius = r0 + (r1 - r0) * ur;

                    float u = Utility::Range(-1.f, 1.f);
                    float theta = Utility::Range(0.f, XM_2PI);
                    float s = sqrtf(1.f - u * u);

                    offset.x = radius * s * cosf(theta);
                    offset.y = radius * u;
                    offset.z = radius * s * sinf(theta);

                    spawnPos.x += offset.x;
                    spawnPos.y += offset.y;
                    spawnPos.z += offset.z;
                }

                // 2) 방향 계산
                _float3 dir{};

                if (spawnData.velFromPos && (offset.x != 0.f || offset.y != 0.f || offset.z != 0.f))
                {
                    // 위치 기준 바깥 방향
                    float len2 = offset.x * offset.x + offset.y * offset.y + offset.z * offset.z;
                    float inv = 1.f / sqrtf(len2);

                    dir.x = offset.x * inv;
                    dir.y = offset.y * inv;
                    dir.z = offset.z * inv;
                }
                else
                {
                    // 기존 baseDir + spreadAng + basis
                    _float3 baseDir = spawnData.baseDir;

                    if (spawnData.dirLocal && spawner.useBasis)
                    {
                        dir.x = baseDir.x * spawner.basisRight.x + baseDir.y * spawner.basisUp.x + baseDir.z * spawner.basisForward.x;
                        dir.y = baseDir.x * spawner.basisRight.y + baseDir.y * spawner.basisUp.y + baseDir.z * spawner.basisForward.y;
                        dir.z = baseDir.x * spawner.basisRight.z + baseDir.y * spawner.basisUp.z + baseDir.z * spawner.basisForward.z;
                    }
                    else
                        dir = baseDir;

                    const float angle = (rSpread * 2.f - 1.f) * spawnData.spreadAng;

                    _vec vDir = XMLoadFloat3(&dir);
                    _vec up = spawner.useBasis
                        ? XMLoadFloat3(&spawner.basisUp)
                        : XMVectorSet(0.f, 1.f, 0.f, 0.f);

                    _mat rot = XMMatrixRotationAxis(up, angle);
                    vDir = XMVector3TransformNormal(vDir, rot);
                    XMStoreFloat3(&dir, vDir);
                }

                const int idx = aliveCount;
                Particle& particle = particles[static_cast<size_t>(idx)];
                ++aliveCount;

                particle.data = &spawnData;
                particle.texKey = spawnData.texKey;
                particle.aliveTime = 0.f;
                particle.lifeTime = lifeTime;
                particle.pos = spawnPos;
                particle.velocity.x = dir.x * speed;
                particle.velocity.y = dir.y * speed;
                particle.velocity.z = dir.z * speed;
                particle.size = spawnData.startSize;
                particle.color = spawnData.startColor;
                particle.owner = spawner.owner;

                if (spawnData.randomStartRot)
                    particle.rotRad = Utility::Range(0.f, XM_2PI);
                else
                    particle.rotRad = 0.f;

                if (spawnData.rotSpeedMin != 0.f || spawnData.rotSpeedMax != 0.f)
                {
                    const float rRot = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                    particle.rotSpeed =
                        spawnData.rotSpeedMin + (spawnData.rotSpeedMax - spawnData.rotSpeedMin) * rRot;
                }
                else
                    particle.rotSpeed = 0.f;

                particle.sheetFrame = 0;
                if (spawnData.sheet.enabled)
                    particle.sheetFrame = spawnData.sheet.startFrame;
            }
        });
}