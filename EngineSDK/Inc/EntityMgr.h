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
	bool IsAlive(EntityID id) const { return alive.count(id) > 0; }
	void Reserve(size_t n)          { freeList.reserve(n); }

	template<typename Func>
	void ForEachAlive(Func&& func) const
	{
		for (auto id : alive) 
			func(id);
	}

	void Clear();

private:
	vector<EntityID>        freeList;
	unordered_set<EntityID> alive;
	vector<EntityID>        deferred; // 지연 파괴목록			      	   
	EntityID                nextID = 1;

	SystemRegistry&         registry;
};

NS_END