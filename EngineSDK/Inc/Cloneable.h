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
		newObj->Init_Proto(); // 이 객체가 어떤 컴포넌트들로 구성되는지 뼈대를 정의

		for (const auto& [typeId, protoComp] : this->comps)
		{
			Obj* newObjBase = newObj.get();
			Component* newComp = newObjBase->GetComp(typeId);

			if (newComp)
				newComp->CopyFrom(protoComp.get());
		}

		newObj->Init(arg); // 인스턴스별 최종 설정
		return newObj;
	}
};

NS_END