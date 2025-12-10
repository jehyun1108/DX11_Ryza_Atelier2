#include "Enginepch.h"

SystemRegistry::SystemRegistry()
{
	table.assign(kSystemCount, nullptr);
}

void SystemRegistry::BootAll()
{
	for (auto* sys : allSystems)
		sys->OnBoot();
}

void SystemRegistry::Reserve(size_t n)
{
	for (auto* r : reservables)
		r->Reserve(n);
}

void SystemRegistry::DestroyOwned(EntityID owner)
{
	for (auto* sys : owningSystems)
		sys->DestroyOwned(owner);
}

void SystemRegistry::Clear()
{
	table.assign(kSystemCount, nullptr);
	owningSystems.clear();
	guiSystems.clear();
	reservables.clear();
	allSystems.clear();
	externalSystems.clear();
	ownedSystems.clear();
}