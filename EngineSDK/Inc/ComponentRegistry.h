#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL ComponentRegistry
{
public:
	static ComponentRegistry& Instance()
	{
		static ComponentRegistry instance;
		return instance;
	}

	void QueueRegister(Component* comp);
	void QueueUnregister(Component* comp);

	void BeginFrame() {}
	void EndFrame() {}

	template<typename Fn>
	void ForEach(COMPONENT type, Fn&& fn)
	{
		auto& vec = buckets[(size_t)type];
		for (Component* comp : vec)
			fn(comp);
	}

	void FlushPending();

private:
	vector<Component*> buckets[(size_t)COMPONENT::END];
	vector<Component*> pendingAdds;
	vector<Component*> pendingRemoves;
};

NS_END