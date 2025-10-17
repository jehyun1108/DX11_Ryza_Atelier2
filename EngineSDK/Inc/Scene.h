#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL Scene abstract : public Base
{
protected:
	Scene() = default;
	virtual ~Scene() = default;

public:
	template<typename T>
	static Scene* Create()
	{
		return new T;
	}

	virtual void Load() = 0;
	virtual void Update(float dt) = 0;
	virtual void LateUpdate(float dt) = 0;
	virtual void UnLoad() = 0;
};

NS_END