#pragma once

#include "IOwnsEntities.h"
#include "IGuiRenderable.h"
#include "IReservable.h"
#include "IModeOrchestra.h"

NS_BEGIN(Engine)

class SystemRegistry
{
public:
	template<typename T, typename...Args>
	T& Emplace(Args&&...args);
	template<typename T>
	void RegisterRef(T& system);

	template<typename T>
	T& Get() const;
	template<typename T>
	T* TryGet() const;

	template<typename TypeList>
	void EmplaceAll();
	template<typename TypeList>
	void BootAllTyped();

	void BootAll();
	void Reserve(size_t n);
	void DestroyOwned(EntityID owner);
	void Clear();

	const vector<IGuiRenderable*>& GetGuiSystems() const { return guiSystems; }

private:
	template<typename T>
	void RegisterCommon(T& system);

private:
	unordered_map<type_index, void*> table;

	vector<IOwnsEntities*>      owningSystems;
	vector<IGuiRenderable*>     guiSystems;
	vector<IReservable*>        reservables;

	vector<ISystem*>            allSystems;
	vector<ISystem*>            externalSystems;
	vector<unique_ptr<ISystem>> ownedSystems;
};
// ====================================================================================================================

template<typename T, typename...Args>
T& SystemRegistry::Emplace(Args&&...args)
{
	static_assert(is_base_of_v<ISystem, T>);
	auto raw = make_unique<T>(*this, forward<Args>(args)...);
	T& ref = *raw;
	RegisterCommon(ref);

	unique_ptr<ISystem> basePtr(static_cast<ISystem*>(raw.release()));
	ownedSystems.push_back(move(basePtr));
	return ref;
}

template<typename T>
void SystemRegistry::RegisterRef(T& system)
{
	RegisterCommon(system);
	if constexpr (is_base_of_v<ISystem, T>)
		externalSystems.push_back(static_cast<ISystem*>(addressof(system)));
}

template<typename T>
T& SystemRegistry::Get() const
{
	auto it = table.find(type_index(typeid(T)));
	if (it == table.end()) throw runtime_error("System not registered");
	return *static_cast<T*>(it->second);
}

template<typename T>
T* SystemRegistry::TryGet() const
{
	auto it = table.find(type_index(typeid(T)));
	return (it == table.end()) ? nullptr : static_cast<T*>(it->second);
}

template<typename TypeList>
void SystemRegistry::EmplaceAll()
{
	static_assert(AllDerived<ISystem, TypeList>::value, "EmplaceAll<TL> requires all TL types derive from ISystem");
	detail::ForEachType(TypeList{}, [this]<typename T>() { this->Emplace<T>(); });
}

template<typename TypeList>
void SystemRegistry::BootAllTyped()
{
	static_assert(AllDerived<ISystem, TypeList>::value, "BootAllTyped<TL> requires all TL types derive from ISystem");
	detail::ForEachType(TypeList{}, [this]<typename T>() { this->Get<T>().OnBoot(); });
}

template<typename T>
void SystemRegistry::RegisterCommon(T& system)
{
	auto tid = type_index(typeid(T));
	auto ok = table.emplace(tid, &system).second;
	if (!ok) 
		throw runtime_error("System already registered");

	if constexpr (is_base_of_v<IOwnsEntities, T>)
		owningSystems.push_back(static_cast<IOwnsEntities*>(&system));
	if constexpr (is_base_of_v<IGuiRenderable, T>)
		guiSystems.push_back(static_cast<IGuiRenderable*>(&system));
	if constexpr (is_base_of_v<IReservable, T>)
		reservables.push_back(static_cast<IReservable*>(&system));
	if constexpr (is_base_of_v<ISystem, T>)
		allSystems.push_back(static_cast<ISystem*>(&system));
}

NS_END