#pragma once

NS_BEGIN(Engine)

struct ENGINE_DLL ISystem
{
	virtual ~ISystem() = default;
	virtual void OnBoot() {}
};
// ====================================================================================================
template<typename... Ts>
struct TypeList {};

template<typename Base, typename TypeList> 
struct AllDerived;

template<typename Base, typename... Ts>
struct AllDerived<Base, TypeList<Ts...>> : bool_constant < (is_base_of_v<Base, Ts> &&...)> {};

template<typename TypeList1, typename TypeList2>
struct TypeListCat;

template<typename...A, typename...B>
struct TypeListCat<TypeList<A...>, TypeList<B...>> { using type = TypeList<A..., B...>; };

template<typename T, typename TypeList>
struct Contains;

template<typename T>
struct Contains<T, TypeList<>> : false_type {};

template<typename T, typename U, typename...Rest>
struct Contains<T, TypeList<U, Rest...>> : conditional_t<is_same_v<T, U>, true_type, Contains<T, TypeList<Rest...>>> {};

template<typename T, typename TypeList>
struct IndexOf;

template<typename T, typename...Ts>
struct IndexOf<T, TypeList<T, Ts...>> : integral_constant<size_t, 0> {};

template<typename T, typename U, typename...Ts>
struct IndexOf<T, TypeList<U, Ts...>> : integral_constant<size_t, 1 + IndexOf<T, TypeList<Ts...>>::value> {};

template<typename TypeList>
struct TypeListSize;

template<typename...Ts>
struct TypeListSize<TypeList<Ts...>> : integral_constant<size_t, sizeof...(Ts)> {};

// ========================================================================================================
namespace detail
{
	template<typename Func>
	inline void ForEachType(TypeList<>, Func&&) {}

	template<typename T, typename...Rest, typename Func>
	inline void ForEachType(TypeList<T, Rest...>, Func&& func)
	{
		func.template operator() < T > ();
		ForEachType(TypeList<Rest...>{}, forward<Func>(func));
	}
}

NS_END