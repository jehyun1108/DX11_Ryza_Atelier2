#pragma once

NS_BEGIN(Engine)

template<typename T>
class SlotPool
{
public:
	Handle CreateSlot(_uint owner = 0);
	void   DestroySlot(Handle handle);
	bool   Validate(Handle handle) const;

	T*       Get(Handle handle);
	const T* Get(Handle handle) const;

	void  Reserve(size_t n);
	_uint GetOwner(Handle handle) const;

	void DestroyOwned(_uint owner);

	template<typename Fn>
	void ForEachAlive(Fn&& fn);
	template<typename Fn>
	void ForEachAlive(Fn&& fn) const;
	
	template<typename Fn>
	void ForEachOwned(_uint owner, Fn&& fn);
	template<typename Fn>
	void ForEachOwned(_uint owner, Fn&& fn) const;

	template<typename Fn>
	void ForEachAliveEx(Fn&& fn);
	template<typename Fn>
	void ForEachAliveEx(Fn&& fn) const;

private:
	vector<T>     data;
	vector<_uint> generations;
	vector<_uint> owners;
	vector<_uint> freeList;
};

template<typename T>
inline Handle SlotPool<T>::CreateSlot(_uint owner)
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
	assert(owners[idx] != 0 && "CreateSlot: owner si 0 (invalidEntity)");
	return Handle{ idx, generations[idx] };
}

template<typename T>
inline void SlotPool<T>::DestroySlot(Handle handle)
{
	if (!Validate(handle)) return;
	const _uint idx = handle.idx;
	owners[idx] = 0;
	++generations[idx];
	freeList.push_back(idx);
}

template<typename T>
inline bool SlotPool<T>::Validate(Handle handle) const
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
inline _uint SlotPool<T>::GetOwner(Handle handle) const
{
	return Validate(handle) ? owners[handle.idx] : 0;
}

template<typename T>
inline void SlotPool<T>::DestroyOwned(_uint owner)
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
inline void SlotPool<T>::Reserve(size_t n)
{
	data.reserve(n);
	generations.reserve(n);
	owners.reserve(n);
	freeList.reserve(n);
}

template<typename T>
inline T* SlotPool<T>::Get(Handle handle)
{
	return Validate(handle) ? &data[handle.idx] : nullptr;
}

template<typename T>
inline const T* SlotPool<T>::Get(Handle handle) const
{
	return Validate(handle) ? &data[handle.idx] : nullptr;
}

template<typename T>
template<typename Fn>
inline void SlotPool<T>::ForEachAlive(Fn&& fn)
{
	for (_uint i = 0; i < (_uint)data.size(); ++i)
	{
		if (i < owners.size() && owners[i] != 0)
			fn(i, data[i]);
	}
}

template<typename T>
template<typename Fn>
inline void SlotPool<T>::ForEachAlive(Fn&& fn) const
{
	for (_uint i = 0; i < (_uint)data.size(); ++i)
	{
		if (i < owners.size() && owners[i] != 0)
			fn(i, data[i]);
	}
}

template<typename T>
template<typename Fn>
inline void SlotPool<T>::ForEachOwned(_uint owner, Fn&& fn)
{
	for (_uint i = 0; i < (_uint)data.size(); ++i)
	{
		if (i < owners.size() && owners[i] == owner)
		{
			Handle handle{ i, generations[i] };
			fn(handle, data[i]);
		}
	}
}

template<typename T>
template<typename Fn>
inline void SlotPool<T>::ForEachOwned(_uint owner, Fn&& fn) const
{
	for (_uint i = 0; i < (_uint)data.size(); ++i)
	{
		if (i < owners.size() && owners[i] == owner)
		{
			Handle handle{ i, generations[i] };
			fn(handle, data[i]);
		}
	}
}

template<typename T>
template<typename Fn>
inline void SlotPool<T>::ForEachAliveEx(Fn&& fn)
{
	for (_uint i = 0; i < (_uint)data.size(); ++i)
	{
		if (i < owners.size() && owners[i] != 0)
			fn(Handle{ i, generations[i] }, owners[i], data[i]);
	}
}

template<typename T>
template<typename Fn>
inline void SlotPool<T>::ForEachAliveEx(Fn&& fn) const
{
	for (_uint i = 0; i < (_uint)data.size(); ++i)
	{
		if (i < owners.size() && owners[i] != 0)
			fn(Handle{ i, generations[i] }, owners[i], data[i]);
	}
}

NS_END