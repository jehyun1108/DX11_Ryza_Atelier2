#pragma once

#include "IOwnsEntities.h"
#include "IGuiRenderable.h"
#include "IReservable.h"

NS_BEGIN(Engine)

class SystemRegistry
{
public:
	template<typename T>
	void Register(T& system)
	{
		table[typeid(T)] = &system;
		
		if constexpr (is_base_of_v<IOwnsEntities, T>)
			owningSystems.push_back(static_cast<IOwnsEntities*>(&system));

		if constexpr (is_base_of_v<IGuiRenderable, T>)
			guiSystems.push_back(static_cast<IGuiRenderable*>(&system));

		if constexpr (is_base_of_v<IReservable, T>)
			reservables.push_back(static_cast<IReservable*>(&system));
	}

	void DestroyOwned(EntityID owner)
	{
		for (auto system : owningSystems)
			system->DestroyOwned(owner);
	}

	template<typename T>
	T& Get() const
	{
		auto it = table.find(typeid(T));
		if (it == table.end()) throw runtime_error("System not registered");
		return *static_cast<T*>(it->second);
	}

	template<typename T>
	T* TryGet() const
	{
		auto it = table.find(typeid(T));
		return (it == table.end()) ? nullptr : static_cast<T*>(it->second);
	}

	const vector<IGuiRenderable*>& GetGuiSystems() const { return guiSystems; }

	void Reserve(size_t n)
	{
		for (auto* r : reservables)
			r->Reserve(n);
	}

	void Clear()
	{
		table.clear();
		owningSystems.clear();
		guiSystems.clear();
		reservables.clear();
	}

private:
	unordered_map<type_index, void*> table;
	vector<IOwnsEntities*>  owningSystems;
	vector<IGuiRenderable*> guiSystems;
	vector<IReservable*>    reservables;
};

NS_END