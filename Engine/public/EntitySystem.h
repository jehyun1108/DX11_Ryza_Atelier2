#pragma once

NS_BEGIN(Engine)

template<typename T>
class EntitySystem : public IOwnsEntities, public IReservable
{
public:
	explicit EntitySystem(SystemRegistry& registry) : registry(registry) {}

	void     Reserve(size_t n) override    { pool.Reserve(n); }
	bool     Validate(Handle handle) const { return pool.Validate(handle); }

	EntityID GetOwner(Handle handle) const { return pool.GetOwner(handle); }
	T*       GetByOwner(EntityID owner, Handle* outHandle = nullptr);
	const T* GetByOwner(EntityID owner, Handle* outHandle = nullptr) const;

	T*       Get(Handle handle)        { return pool.Get(handle); }
	const T* Get(Handle handle)  const { return pool.Get(handle); }

	void Destroy(Handle handle)                  { pool.DestroySlot(handle); }
	void DestroyOwned(EntityID owner)  override  { pool.DestroyOwned(owner); }

	// 모든 살아있는 슬롯 (owner, handle, data) 같이 오너와 정보까지 포함해 순회
	template<typename Func>
	void ForEachAliveEx(Func&& func)       { pool.ForEachAliveEx(forward<Func>(func)); }
	template<typename Func>
	void ForEachAliveEx(Func&& func) const { pool.ForEachAliveEx(forward<Func>(func)); }

	// 특정 Entity가 소유한 컴포넌트만 순회
	template<typename Func>
	void ForEachOwned(_uint owner, Func&& func)       { pool.ForEachOwned(owner, forward<Func>(func)); }
	template<typename Func>
	void ForEachOwned(_uint owner, Func&& func) const { pool.ForEachOwned(owner, forward<Func>(func)); }

protected:
	Handle CreateComp(EntityID owner) { return pool.CreateComp(owner); }

	SystemRegistry&  registry;
	ComponentPool<T> pool;
};

template<typename T>
inline T* EntitySystem<T>::GetByOwner(EntityID owner, Handle* outHandle)
{
	Handle handle{};
	T* ptr{};
	if (pool.FindOwned(owner, handle, ptr))
		{
			if (outHandle)
				*outHandle  = handle;

			return ptr;
		}
	return nullptr;
}

template<typename T>
inline const T* EntitySystem<T>::GetByOwner(EntityID owner, Handle* outHandle) const
{
	Handle handle{};
	const T* ptr{};
	if (pool.FindOwned(owner, handle, ptr))
	{
		if (outHandle)
			*outHandle = handle;
		
		return ptr;
	}
	return nullptr;
}

NS_END
