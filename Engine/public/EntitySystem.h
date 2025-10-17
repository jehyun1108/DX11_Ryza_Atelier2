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

	void DestroySlot(Handle handle)              { pool.DestroySlot(handle); }
	void DestroyOwned(EntityID owner)  override  { pool.DestroyOwned(owner); }

	// owner/handle ��� �Ǵ� ���� ������Ʈ��
	template<typename Func>
	void ForEachAlive(Func&& func)       { pool.ForEachAlive(forward<Func>(func));}
	template<typename Func>
	void ForEachAlive(Func&& func) const { pool.ForEachAlive(forward<Func>(func));}

	// ��� ����ִ� ���� (owner, handle, data) ���� ���ʿ� �������� ������ ��ȸ
	template<typename Func>
	void ForEachAliveEx(Func&& func)       { pool.ForEachAliveEx(forward<Func>(func)); }
	template<typename Func>
	void ForEachAliveEx(Func&& func) const { pool.ForEachAliveEx(forward<Func>(func)); }

	// Ư�� Entity�� ������ ������Ʈ�� ��ȸ
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
