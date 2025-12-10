#include "Enginepch.h"

#include "ScreenDistortionSystem.h"

void FieldOrchestraSystem::OnBoot()
{
	uiOrchestrator = &registry.Get<FieldUIOrchestrator>();
	input          = &registry.Get<InputService>();
	fieldCtrlSys   = &registry.Get<FieldControllerSystem>();
	fieldAnimSys   = &registry.Get<FieldAnimSystem>();
	sessionSys     = &registry.Get<BattleSessionSystem>();
	dataSys        = &registry.Get<CharacterDataSystem>();
	director       = &registry.Get<GameModeDirectorSystem>();
	distortionSys  = &registry.Get<ScreenDistortionSystem>();
	collisionSys   = &registry.Get<CollisionSystem>();
	tfSys          = &registry.Get<TransformSystem>();
	layerSys       = &registry.Get<LayerSystem>();
}

void FieldOrchestraSystem::Enter()
{
	input->SetContext(InputContext::Field);
	input->SetFocus(FocusState::None);
	input->SetManualTime(0.f);
	uiOrchestrator->Enter();
}

void FieldOrchestraSystem::Update(float dt)
{
	EntityID leader = dataSys->GetEntityID(CharacterID::Ryza);
	fieldCtrlSys->Update(leader, dt);
	fieldAnimSys->Update(dt);
	uiOrchestrator->Tick(dt);

	BattleHitInfo hit{};
	if (collisionSys->FindWeaponHit(leader, hit))
	{
		BattleStartParams start{};

		start.allies.members[0] = dataSys->GetEntityID(CharacterID::Ryza);
		start.allies.members[1] = dataSys->GetEntityID(CharacterID::Klaudia);
		start.allies.members[2] = dataSys->GetEntityID(CharacterID::Patricia);

		start.allies.memberCount = 0;
		for (int i = 0; i < 3; ++i)
			if (start.allies.members[i] != 0u)
				++start.allies.memberCount;

		start.enemies.members = { 0u, 0u, 0u };
		start.enemies.memberCount = 0;

		vector<EntityID> allEnemies;
		CollectFieldEnemies(allEnemies); 
		BuildEnemiesAroundHit(hit.centerWorld, hit.target, allEnemies, start.enemies);

		start.centerWorld = hit.centerWorld;
		start.startAngleDeg = 0.f;
		start.ringRadius = 400.f;
		start.faceCenterSnap = true;

		distortionSys->StartBattleToField(start.centerWorld);
		director->BeginBattle(start);
	}
}

void FieldOrchestraSystem::Exit()
{
	uiOrchestrator->Exit();
}

void FieldOrchestraSystem::BuildEnemiesAroundHit(const _float3& center, EntityID primaryTarget, const vector<EntityID>& allEnemies, BattleEnemies& outEnemies)
{
	struct Candidate
	{
		EntityID entity;
		float    distSq;
	};

	vector<Candidate> cand;
	cand.reserve(allEnemies.size() + 1);

	const float maxRadius = 1000.f;
	const float maxRadiusSq = maxRadius * maxRadius;

	auto addCandidate = [&](EntityID e)
		{
			if (e == 0u) return;

			Handle tfHandle{};
			tfSys->GetByOwner(e, &tfHandle);
			TransformData* tf = tfSys->Get(tfHandle);
			assert(tf); 

			_float3 d{
				tf->pos.x - center.x,
				tf->pos.y - center.y,
				tf->pos.z - center.z
			};
			float distSq = d.x * d.x + d.y * d.y + d.z * d.z;
			if (distSq <= maxRadiusSq)
				cand.push_back({ e, distSq });
		};

	addCandidate(primaryTarget);

	for (EntityID e : allEnemies)
	{
		if (e == primaryTarget) continue;
		addCandidate(e);
	}

	sort(cand.begin(), cand.end(),
		[](const Candidate& a, const Candidate& b)
		{
			return a.distSq < b.distSq;
		});

	outEnemies.members = { 0u, 0u, 0u };
	outEnemies.memberCount = 0;

	int count = 0;
	for (const auto& c : cand)
	{
		if (count >= 3) break;
		outEnemies.members[count] = c.entity;
		++count;
	}
	outEnemies.memberCount = count;
}

void FieldOrchestraSystem::CollectFieldEnemies(vector<EntityID>& out)
{
	out.clear();

	constexpr _uint monsterMask = LayerUtil::LayerBit(LAYER::MONSTER);

	layerSys->ForEachByMask(monsterMask,
		[&](EntityID owner, Handle handle, const LayerData& layer)
		{
			out.push_back(owner);
		});
}