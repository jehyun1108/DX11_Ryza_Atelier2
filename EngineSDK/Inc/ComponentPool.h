#pragma once

NS_BEGIN(Engine)

template<typename T>
class ComponentPool
{
public:
	Handle CreateComp(_uint owner = 0);
	void   DestroySlot(Handle handle);
	bool   Validate(Handle handle) const;

	T*       Get(Handle handle);
	const T* Get(Handle handle) const;

	void  Reserve(size_t n);
	_uint GetOwner(Handle handle) const;

	void DestroyOwned(_uint owner);

	template<typename Func>
	void ForEachAlive(Func&& func);
	template<typename Func>
	void ForEachAlive(Func&& func) const;
	
	template<typename Func>
	void ForEachOwned(_uint owner, Func&& func);
	template<typename Func>
	void ForEachOwned(_uint owner, Func&& func) const;

	template<typename Func>
	void ForEachAliveEx(Func&& func);
	template<typename Func>
	void ForEachAliveEx(Func&& func) const;

	template<typename T1>
	bool FindOwned(_uint owner, Handle& outHandle, T1*& outPtr);
	template<typename T1>
	bool FindOwned(_uint owner, Handle& outHandle, T1*& outPtr) const;

private:
	vector<T>     data;
	vector<_uint> generations;
	vector<_uint> owners;
	vector<_uint> freeList;
};

template<typename T>
inline Handle ComponentPool<T>::CreateComp(_uint owner)
{
	_uint idx;
	if (!freeList.empty()) 
	{
		idx = freeList.back();         freeList.pop_back();
		if (idx >= generations.size()) generations.resize(idx + 1, 0);
		if (idx >= owners.size())      owners.resize(idx + 1, 0);
		if (idx >= data.size())        data.resize(idx + 1);
		++generations[idx];
	}
	else 
	{
		idx = (_uint)data.size();
		data.emplace_back();
		generations.push_back(0);
		owners.push_back(0);
	}
	owners[idx] = owner;
	assert(owners[idx] != 0 && "CreateSlot: owner is 0 (invalidEntity)");
	return Handle{ idx, generations[idx] };
}

template<typename T>
inline void ComponentPool<T>::DestroySlot(Handle handle)
{
	if (!Validate(handle)) return;
	const _uint idx = handle.idx;
	owners[idx] = 0;
	++generations[idx];
	freeList.push_back(idx);
}

template<typename T>
inline bool ComponentPool<T>::Validate(Handle handle) const
{
	if (!handle.IsValid()) return false;
	if (handle.idx >= data.size()) return false;
	if (handle.idx >= generations.size()) return false;
	if (handle.idx >= owners.size()) return false;
	if (generations[handle.idx] != handle.generation) return false;
	if (owners[handle.idx] == 0) return false; 
	return true;
}

template<typename T>
inline _uint ComponentPool<T>::GetOwner(Handle handle) const
{
	return Validate(handle) ? owners[handle.idx] : 0;
}

template<typename T>
inline void ComponentPool<T>::DestroyOwned(_uint owner)
{
	vector<_uint> toKill;
	toKill.reserve(16);
	for (_uint i = 0; i < owners.size(); ++i)
	{
		if (owners[i] == owner)
			toKill.push_back(i);
	}
	// 파괴 -> 새대수 증가 & freeList 처리
	for (auto idx : toKill)
		DestroySlot(Handle{ idx, generations[idx] });
}

template<typename T>
inline void ComponentPool<T>::Reserve(size_t n)
{
	data.reserve(n);
	generations.reserve(n);
	owners.reserve(n);
	freeList.reserve(n);
}

template<typename T>
inline T* ComponentPool<T>::Get(Handle handle)
{
	return Validate(handle) ? &data[handle.idx] : nullptr;
}

template<typename T>
inline const T* ComponentPool<T>::Get(Handle handle) const
{
	return Validate(handle) ? &data[handle.idx] : nullptr;
}

template<typename T>
template<typename Func>
inline void ComponentPool<T>::ForEachAlive(Func&& func)
{
	for (_uint i = 0; i < (_uint)data.size(); ++i)
	{
		if (i < owners.size() && owners[i] != 0)
			func(i, data[i]);
	}
}

template<typename T>
template<typename Func>
inline void ComponentPool<T>::ForEachAlive(Func&& func) const
{
	for (_uint i = 0; i < (_uint)data.size(); ++i)
	{
		if (i < owners.size() && owners[i] != 0)
			func(i, data[i]);
	}
}

template<typename T>
template<typename Func>
inline void ComponentPool<T>::ForEachOwned(_uint owner, Func&& func)
{
	for (_uint i = 0; i < (_uint)data.size(); ++i)
	{
		if (i < owners.size() && owners[i] == owner)
		{
			Handle handle{ i, generations[i] };
			func(handle, data[i]);
		}
	}
}

template<typename T>
template<typename Func>
inline void ComponentPool<T>::ForEachOwned(_uint owner, Func&& func) const
{
	for (_uint i = 0; i < (_uint)data.size(); ++i)
	{
		if (i < owners.size() && owners[i] == owner)
		{
			Handle handle{ i, generations[i] };
			func(handle, data[i]);
		}
	}
}

template<typename T>
template<typename Func>
inline void ComponentPool<T>::ForEachAliveEx(Func&& func)
{
	for (_uint i = 0; i < (_uint)data.size(); ++i)
	{
		if (i < owners.size() && owners[i] != 0)
			func(Handle{ i, generations[i] }, owners[i], data[i]);
	}
}

template<typename T>
template<typename Func>
inline void ComponentPool<T>::ForEachAliveEx(Func&& func) const
{
	for (_uint i = 0; i < (_uint)data.size(); ++i)
	{
		if (i < owners.size() && owners[i] != 0)
			func(Handle{ i, generations[i] }, owners[i], data[i]);
	}
}

template<typename T>
template<typename T1>
inline bool ComponentPool<T>::FindOwned(_uint owner, Handle& outHandle, T1*& outPtr)
{
	for (_uint i = 0; i < (_uint)data.size(); ++i)
	{
		if (i < owners.size() && owners[i] == owner)
		{
			outHandle = Handle{ i, generations[i] };
			outPtr = &data[i];
			return true;
		}
	}
	return false;
}

template<typename T>
template<typename T1>
inline bool ComponentPool<T>::FindOwned(_uint owner, Handle& outHandle, T1*& outPtr) const
{
	for (_uint i = 0; i < (_uint)data.size(); ++i)
	{
		if (i < owners.size() && owners[i] == owner)
		{
			outHandle = Handle{ i, generations[i] };
			outPtr = &data[i];
			return true;
		}
	}
	return false;
}

NS_END