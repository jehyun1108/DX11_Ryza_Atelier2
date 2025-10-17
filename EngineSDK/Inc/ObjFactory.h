#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL ObjFactory final
{
public:
	template<typename T>
	void Register(const wstring& type)
	{
		// 타입 이름과, 해당 타입의 비어있는 객체를 생성하는 람다 함수를 맵에 저장
		creators[type] = []() {return make_unique<T>(); };
	}

	unique_ptr<Obj> CreateObj(const wstring& type)
	{
		auto it = creators.find(type);
		if (it == creators.end())
		{
			assert(false && "등록되지 않은 오브젝트 타입입니다.");
			return nullptr;
		}
		// 등록된 생성 함수를 호출하여 객체 생성
		return it->second();
	}

public:
	static unique_ptr<ObjFactory> Create() { return make_unique<ObjFactory>(); }

private:
	map<wstring, function<unique_ptr<Obj>()>> creators;
};

NS_END