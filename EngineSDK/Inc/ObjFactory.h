#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL ObjFactory final
{
public:
	template<typename T>
	void Register(const wstring& type)
	{
		// Ÿ�� �̸���, �ش� Ÿ���� ����ִ� ��ü�� �����ϴ� ���� �Լ��� �ʿ� ����
		creators[type] = []() {return make_unique<T>(); };
	}

	unique_ptr<Obj> CreateObj(const wstring& type)
	{
		auto it = creators.find(type);
		if (it == creators.end())
		{
			assert(false && "��ϵ��� ���� ������Ʈ Ÿ���Դϴ�.");
			return nullptr;
		}
		// ��ϵ� ���� �Լ��� ȣ���Ͽ� ��ü ����
		return it->second();
	}

public:
	static unique_ptr<ObjFactory> Create() { return make_unique<ObjFactory>(); }

private:
	map<wstring, function<unique_ptr<Obj>()>> creators;
};

NS_END