#include "Enginepch.h"

void FieldOrchestraSystem::OnBoot()
{
	uiOrchestrator = &registry.Get<FieldUIOrchestrator>();
	input          = &registry.Get<InputService>();
	fieldCtrlSys   = &registry.Get<FieldControllerSystem>();
	fieldAnimSys   = &registry.Get<FieldAnimSystem>();
	sessionSys     = &registry.Get<BattleSessionSystem>();
	dataSys        = &registry.Get<CharacterDataSystem>();
	director       = &registry.Get<GameModeDirectorSystem>();

	assert(uiOrchestrator && input && fieldCtrlSys && fieldAnimSys && sessionSys && dataSys && director);
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
	fieldCtrlSys->Update(1, dt);
	fieldAnimSys->Update(dt);
	uiOrchestrator->Tick(dt);

	const bool isDown = input->KeyPressing(KEY::NUM1);
	const bool isEdge = (isDown && !prevBattleKeyDown);
	prevBattleKeyDown = isDown;

	if (isEdge)
		BeginBattle(); 
}

void FieldOrchestraSystem::Exit()
{
	uiOrchestrator->Exit();
}

void FieldOrchestraSystem::BeginBattle()
{
	BattleStartParams start{};
	start.allies.members[0]  = dataSys->GetEntityID(CharacterID::Ryza);
	start.allies.members[1]  = dataSys->GetEntityID(CharacterID::Klaudia);
	start.allies.members[2]  = dataSys->GetEntityID(CharacterID::Patricia);
	start.allies.memberCount = 0;
	for (int i = 0; i < 3; ++i)
		if (start.allies.members[i] != invalidEntity) ++start.allies.memberCount;
	// ----------------------------------------------------------------
	vector<_uint> angelEntities = dataSys->GetEntities(CharacterID::Angel);

	start.enemies.members = { invalidEntity, invalidEntity, invalidEntity };
	start.enemies.memberCount = 0;

	const size_t maxToFill = min<size_t>(3, angelEntities.size());
	int filledCount = 0;
	for (size_t i = 0; i < maxToFill && filledCount < 3; ++i)
	{
		const EntityID enemyEntity = angelEntities[i];
		if (enemyEntity == invalidEntity) continue; 

		start.enemies.members[filledCount] = enemyEntity;
		++filledCount;
	}
	start.enemies.memberCount = filledCount;
	// -----------------------------------------------
	start.centerWorld    = _float3{};
	start.startAngleDeg  = 0.f;
	start.ringRadius     = 600.f;
	start.faceCenterSnap = true;

	director->BeginBattle(start);
}