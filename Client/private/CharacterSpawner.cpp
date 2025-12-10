#include "pch.h"
#include "CharacterSpawner.h"

CharacterSpawner::CharacterSpawner(SystemRegistry& registry) : registry(registry)
{
	spawner = &registry.Get<EntitySpawner>();
	dataSys = &registry.Get<CharacterDataSystem>();
	socket  = &registry.Get<SocketSystem>();
}

EntityHandles CharacterSpawner::SpawnRyza(const _float3& pos)
{
	auto ryza = spawner->NewEntity()
		.WithTf(TransformDesc{ .pos = pos })
		.WithLayer(LayerUtil::LayerBit(LAYER::PLAYER))
		.WithModel(L"ryza")
		.WithColliderFromModel(ColliderType::OBB, Bit(CollisionLayer::Character), Bit(CollisionLayer::Character) | Bit(CollisionLayer::Ground) | Bit(CollisionLayer::Prop) | Bit(CollisionLayer::Trigger), true)
		.WithTag("ryza")
		.WithPlayerMovement()
#ifdef NDEBUG
		//.WithFace(L"PC20A_Face_Eye_UP", L"PC20A_Face_CloseEye_down")
		//.WithMouth(L"PC20A_Face_pronunciation")
#endif
		.Build();

	dataSys->BindEntity(ryza.entity, CharacterID::Ryza);

	auto ryza_weapon = spawner->NewEntity()
		.WithTf()
		.WithLayer(LayerUtil::LayerBit(LAYER::SOCKET))
		.WithModel(L"ryza_weapon_4")
		.WithColliderFromModel(ColliderType::OBB, Bit(CollisionLayer::Trigger), Bit(CollisionLayer::Character), true)
		.WithTag("ryza_weapon_4")
		.WithSocket("ryza", "bone_63", _float3(1.f, 3.f, 0.f), _float3(0.f, 90.f, 0.f))
		.Build();

	auto ryza_cap = spawner->NewEntity()
		.WithTf()
		.WithLayer(LayerUtil::LayerBit(LAYER::SOCKET))
		.WithModel(L"ryza_cap")
		.WithTag("ryza_cap")
		.WithSocket("ryza", "bone_961", _float3(-4.f, -153.f, 0.f), _float3(0, -90.f, 0.f))
		.Build();

	return ryza;
}

EntityHandles CharacterSpawner::SpawnKlaudia(const _float3& pos)
{
	auto klaudia = spawner->NewEntity()
		.WithTf(TransformDesc{ .pos = pos })
		.WithLayer(LayerUtil::LayerBit(LAYER::PLAYER))
		.WithModel(L"klaudia")
		//.WithColliderFromModel(ColliderType::OBB, Bit(CollisionLayer::Character), Bit(CollisionLayer::Character) | Bit(CollisionLayer::Ground) | Bit(CollisionLayer::Prop) | Bit(CollisionLayer::Trigger), true)
		.WithTag("klaudia")
#ifdef NDEBUG
		//.WithFace(L"PC21A_Face_Eye_UP", L"PC21A_Face_CloseEye_down")
		//.WithMouth(L"PC21A_Face_pronunciation")
#endif
		.WithPlayerMovement()
		.Build();

	dataSys->BindEntity(klaudia.entity, CharacterID::Klaudia);

	auto weapon = spawner->NewEntity()
		.WithTf()
		.WithLayer(LayerUtil::LayerBit(LAYER::SOCKET))
		.WithModel(L"klaudia_weapon_2")
		//.WithColliderFromModel(ColliderType::OBB, Bit(CollisionLayer::Trigger), Bit(CollisionLayer::Character), true)
		.WithTag("kluadia_weapon_2")
		.WithSocket("klaudia", "bone_62", _float3(2.f, 2.5f, 0.f), _float3(-103.f, 45.f, 30.f))
		.Build();

	return klaudia;
}

EntityHandles CharacterSpawner::SpawnPatricia(const _float3& pos)
{
	auto patricia = spawner->NewEntity()
		.WithTf(TransformDesc{ .pos = pos })
		.WithLayer(LayerUtil::LayerBit(LAYER::PLAYER))
		.WithModel(L"patricia")
		//.WithColliderFromModel(ColliderType::OBB, Bit(CollisionLayer::Character), Bit(CollisionLayer::Character) | Bit(CollisionLayer::Ground) | Bit(CollisionLayer::Prop) | Bit(CollisionLayer::Trigger), true)
#ifdef NDEBUG
		//.WithFace(L"PC24A_Face_Eye_UP", L"PC24A_Face_CloseEye_down")
		//.WithMouth(L"PC24A_Face_pronunciation")
#endif
		.WithTag("patricia")
		.WithPlayerMovement()
		.Build();

	dataSys->BindEntity(patricia.entity, CharacterID::Patricia);

	auto patricia_weapon = spawner->NewEntity()
		.WithTf()
		.WithLayer(LayerUtil::LayerBit(LAYER::SOCKET))
		.WithTag("patricia_weapon")
		.WithModel(L"patricia_weapon")
		.WithColliderFromModel(ColliderType::OBB, Bit(CollisionLayer::Prop), Bit(CollisionLayer::Character), true)
		.WithSocket("patricia", "bone_71", _float3(3.f, 3.f, 0.f), _float3(257.f, 86.f, -1.4f))
		.Build();

	dataSys->SetCamAnchor(CharacterID::Patricia, patricia.entity);
	//registry.Get<CamRegistry>().SetDebugAnchorTf(patricia.tf);

	return patricia;
}
