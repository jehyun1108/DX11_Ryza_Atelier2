#include "Enginepch.h"

void FieldUIOrchestrator::Enter()
{
	const auto& archetypes = registry.Get<UIRegistry>().GetArchetypes();
	assert(!archetypes.empty() && "UI archetypes must be registered BEFORE FieldUIOrchestrator::Enter()");

	for (auto& [key, spec] : archetypes)
	{
		if (spec.context == UIContext::Field && spec.startEnabled)
			uiRegistry.Ensure(key);
	}
}

void FieldUIOrchestrator::Tick(float dt)
{
	uiAnimSys.Tick(dt);
	uiSys.Tick(dt);
}

void FieldUIOrchestrator::Exit()
{
	for (auto& [key, inst] : uiRegistry.GetInstances())
	{
		const UIArchetypeSpec* spec = inst.spec;
		if (spec && spec->context == UIContext::Field)
			uiRegistry.SetEnabled(key, false);
	}
}