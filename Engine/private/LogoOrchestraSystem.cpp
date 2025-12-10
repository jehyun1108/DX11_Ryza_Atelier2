#include "Enginepch.h"
#include "LogoOrchestraSystem.h"
#include "LogoUIOrchestrator.h"

void LogoOrchestraSystem::OnBoot()
{
	uiOrchestrator = &registry.Get<LogoUIOrchestrator>();
	input          = &registry.Get<InputService>();
}

void LogoOrchestraSystem::Enter()
{
	input->SetContext(InputContext::Menu);  
	input->SetFocus(FocusState::None);
	input->SetManualTime(0.f);

	uiOrchestrator->Enter();
}

void LogoOrchestraSystem::Tick(float dt)
{
	uiOrchestrator->Tick(dt);
}

void LogoOrchestraSystem::Exit()
{
	uiOrchestrator->Exit();
	input->SetFocus(FocusState::None);
}