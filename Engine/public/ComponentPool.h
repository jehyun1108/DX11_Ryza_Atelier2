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
	void ForEachOwned(_uint owner, Func&& func);
	template<typename Func>
	void ForEachOwned(_uint owner, Func&& func) const;

	template<typename Func>
	void ForEachAliveEx(Func&& func);
	template<typename Func>
	void ForEachAliveEx(Func&& func) const;

	bool FindOwned(_uint owner, Handle& outHandle, T*& outPtr);
	bool FindOwned(_uint owner, Handle& outHandle, const T*& outPtr) const;

private:
	vector<T>     data;
	vector<_uint> generations;
	vector<_uint> owners;
	vector<_uint> freeList;

	vector<_uint> aliveIndices; // Dense: 실제 살아있는 idx
	vector<_uint> sparseIndices; // Sparse: idx -> aliveIndices 의 Idx 매핑
};

template<typename T>
inline Handle ComponentPool<T>::CreateComp(_uint owner)
{
	assert(owner != invalidEntity && "CreapComp: owner is invalidEntity");
	_uint idx;
	if (!freeList.empty()) 
	{
		idx = freeList.back();    
		freeList.pop_back();
		++generations[idx];
	}
	else 
	{
		idx = (_uint)data.size();
		data.emplace_back();
		generations.push_back(0);
		owners.push_back(0);
		sparseIndices.push_back(0);
	}
	owners[idx] = owner;
	sparseIndices[idx] = (_uint)aliveIndices.size();
	aliveIndices.push_back(idx);
	return Handle{ idx, generations[idx] };
}

template<typename T>
inline void ComponentPool<T>::DestroySlot(Handle handle)
{
	if (!Validate(handle)) return;
	const _uint idx = handle.idx;
	const _uint IdxtoRemove = sparseIndices[idx];
	const _uint lastIdx = aliveIndices.back();

	aliveIndices[IdxtoRemove] = lastIdx;
	sparseIndices[lastIdx] = IdxtoRemove;
	aliveIndices.pop_back();

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
	for (const _uint idx : aliveIndices)
	{
		if (owners[idx] == owner)
			toKill.push_back(idx);
	}

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
	aliveIndices.reserve(n);
	sparseIndices.reserve(n);
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
inline void ComponentPool<T>::ForEachOwned(_uint owner, Func&& func)
{
	for (const _uint idx : aliveIndices)
	{
		if (owners[idx] == owner)
			func(Handle{ idx, generations[idx] }, data[idx]);
	}
}

template<typename T>
template<typename Func>
inline void ComponentPool<T>::ForEachOwned(_uint owner, Func&& func) const
{
	for (const _uint idx : aliveIndices)
	{
		if (owners[idx] == owner)
			func(Handle{ idx, generations[idx] }, data[idx]);
	}
}

template<typename T>
template<typename Func>
inline void ComponentPool<T>::ForEachAliveEx(Func&& func)
{
	for (const _uint idx : aliveIndices)
		func(Handle{ idx, generations[idx] }, owners[idx], data[idx]);
}

template<typename T>
template<typename Func>
inline void ComponentPool<T>::ForEachAliveEx(Func&& func) const
{
	for (const _uint idx : aliveIndices)
		func(Handle{ idx, generations[idx] }, owners[idx], data[idx]);
}

template<typename T>
inline bool ComponentPool<T>::FindOwned(_uint owner, Handle& outHandle, T*& outPtr)
{
	for (const _uint idx : aliveIndices)
	{
		if (owners[idx] == owner)
		{
			outHandle = Handle{ idx, generations[idx] };
			outPtr = &data[idx];
			return true;
		}
	}
	outPtr = nullptr;
	return false;
}

template<typename T>
inline bool ComponentPool<T>::FindOwned(_uint owner, Handle& outHandle, const T*& outPtr) const
{
	for (const _uint idx : aliveIndices)
	{
		if (owners[idx] == owner)
		{
			outHandle = Handle{ idx, generations[idx] };
			outPtr = &data[idx];
			return true;
		}
	}
	outPtr = nullptr;
	return false;
}

NS_END