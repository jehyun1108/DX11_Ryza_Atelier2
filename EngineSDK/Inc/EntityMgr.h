#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL EntityMgr
{
public:
	explicit EntityMgr(SystemRegistry& registry) : registry(registry) {}

	// ���� & �ı�
	EntityID Create();
	void     Destroy(EntityID id);
	void     DestroyDeferred(EntityID id);
	void     FlushDestroy();

	// ��ƿ
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
	vector<EntityID>        deferred; // ���� �ı����			      	   
	EntityID                nextID = 1;

	SystemRegistry&         registry;
};

NS_END