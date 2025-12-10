#include "pch.h"
#include "EnemySpawner.h"

EnemySpawner::EnemySpawner(SystemRegistry& registry) : registry(registry)
{
	spawner = &registry.Get<EntitySpawner>();
	dataSys = &registry.Get<CharacterDataSystem>();
}

EntityHandles EnemySpawner::SpawnAngel(const _float3& pos)
{
    auto angel = spawner->NewEntity()
        .WithTf(TransformDesc{ .pos = pos })
        .WithEuler(0.f, 180.f, 0.f)
        .WithScale(2.f, 2.f, 2.f)
        .WithLayer(LayerUtil::LayerBit(LAYER::MONSTER))
        .WithModel(L"angel")
        .WithEnemyMovement()
        .WithColliderFromModel(ColliderType::OBB, Bit(CollisionLayer::Character), Bit(CollisionLayer::Character) | Bit(CollisionLayer::Ground) | Bit(CollisionLayer::Prop) | Bit(CollisionLayer::Trigger), true)
        .Build();

    dataSys->BindEntity(angel.entity, CharacterID::Angel);
    return angel;
}

vector<EntityHandles> EnemySpawner::SpawnAngels(const vector<_float3>& positions)
{
    vector<EntityHandles> out;
    out.reserve(positions.size());

    for (const auto& pos : positions)
        out.push_back(SpawnAngel(pos));

    return out;
}
