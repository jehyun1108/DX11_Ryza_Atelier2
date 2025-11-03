#include "Enginepch.h"

void FieldOrchestraSystem::Enter()
{
	auto& uiOrchestrator = registry.Get<FieldUIOrchestrator>();

	auto& input = registry.Get<InputService>();
	input.SetContext(InputContext::Field);
	input.SetFocus(FocusState::None);
	input.SetManualTime(0.f);

	uiOrchestrator.Enter();
}

void FieldOrchestraSystem::Update(float dt)
{
	auto& fieldCtrlSys   = registry.Get<FieldControllerSystem>();
	auto& sessionSys     = registry.Get<BattleSessionSystem>();
	auto& fieldAnimSys   = registry.Get<FieldAnimSystem>();
	auto& input          = registry.Get<InputService>();
	auto& uiOrchestrator = registry.Get<FieldUIOrchestrator>();

	fieldCtrlSys.Update(1, dt);
	fieldAnimSys.Update(dt);

	uiOrchestrator.Tick(dt);

	const bool isDown = input.KeyPressing(KEY::NUM1);
	const bool isEdge = (isDown && !prevBattleKeyDown);
	prevBattleKeyDown = isDown;

	if (isEdge)
		BeginBattle(); 
}

void FieldOrchestraSystem::Exit()
{
	auto& uiOrchestrator = registry.Get<FieldUIOrchestrator>();
	uiOrchestrator.Exit();
}

void FieldOrchestraSystem::BeginBattle()
{
	auto& dataSys = registry.Get<CharacterDataSystem>();

	BattleStartParams start{};
	start.allies.members[0]  = dataSys.GetEntityID(CharacterID::Ryza);
	start.allies.members[1]  = dataSys.GetEntityID(CharacterID::Klaudia);
	start.allies.members[2]  = dataSys.GetEntityID(CharacterID::Patricia);
	start.allies.memberCount = 0;
	for (int i = 0; i < 3; ++i)
		if (start.allies.members[i] != invalidEntity) ++start.allies.memberCount;
	// ----------------------------------------------------------------
	vector<_uint> angelEntities = dataSys.GetEntities(CharacterID::Angel);

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

	registry.Get<GameModeDirectorSystem>().BeginBattle(start);
}