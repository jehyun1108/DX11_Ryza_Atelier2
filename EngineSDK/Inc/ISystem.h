#pragma once

NS_BEGIN(Engine)

struct ENGINE_DLL ISystem
{
	virtual ~ISystem() = default;
	virtual void OnBoot() {}
};

template<typename... Ts> 
struct TypeList {};

template<typename Base, typename TypeList>
struct AllDerived;

template<typename Base, typename... Ts>
struct AllDerived<Base, TypeList<Ts...>> : bool_constant < (is_base_of_v<Base, Ts> &&...)> {};

namespace detail
{
	template<typename Func>
	inline void ForEachType(TypeList<>, Func&&) {}

	template<typename T, typename...Rest, typename Func>
	inline void ForEachType(TypeList<T, Rest...>, Func&& func)
	{
		func.template operator()<T>();
		ForEachType(TypeList<Rest...>{}, forward<Func>(func));
	}
}

NS_END