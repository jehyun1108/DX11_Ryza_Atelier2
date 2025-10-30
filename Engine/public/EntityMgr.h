#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL EntityMgr
{
public:
	explicit EntityMgr(SystemRegistry& registry) : registry(registry) {}

	// 생성 & 파괴
	EntityID Create();
	void     Destroy(EntityID id);
	void     DestroyDeferred(EntityID id);
	void     FlushDestroy();

	// 유틸
	bool IsAlive(EntityID id) const;
	void Reserve(size_t n);

	template<typename Func>
	void ForEachAlive(Func&& func) const
	{
		for (auto id : aliveIndices)
			func(id);
	}

	void Clear();

private:
	static constexpr _uint invalidIdx = 0xFFFFFFFFu;
	EntityID          nextID = 1;
	
	vector<EntityID>  freeList;
	vector<EntityID>  deferred;	      	   
	vector<EntityID> aliveIndices;
	vector<_uint>    sparseIndices;

	SystemRegistry&         registry;
};

NS_END