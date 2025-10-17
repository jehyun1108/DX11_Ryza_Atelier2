#pragma once

#include "Obj.h"

NS_BEGIN(Engine)

template<typename T>
class Cloneable : public Obj
{
public:
	virtual unique_ptr<Obj> Clone(const any& arg) override
	{
		unique_ptr<T> newObj = make_unique<T>();
		newObj->Init_Proto(); // �� ��ü�� � ������Ʈ��� �����Ǵ��� ���븦 ����

		for (const auto& [typeId, protoComp] : this->comps)
		{
			Obj* newObjBase = newObj.get();
			Component* newComp = newObjBase->GetComp(typeId);

			if (newComp)
				newComp->CopyFrom(protoComp.get());
		}

		newObj->Init(arg); // �ν��Ͻ��� ���� ����
		return newObj;
	}
};

NS_END