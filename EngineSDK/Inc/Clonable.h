#pragma once

NS_BEGIN(Engine)

template<typename T>
class Clonable : public Obj
{
public:
	virtual unique_ptr<Obj> Clone(const any& arg = {}) override
	{
		auto newObj = make_unique<T>(static_cast<const T&>(*this));

		newObj->comps.clear();

		for (const auto& [type, protoComp] : this->comps)
		{
			if (protoComp)
				newObj->comps.emplace(type, protoComp->Clone(newObj.get()));
		}

		newObj->Init(arg);
		return newObj;
	}
};

NS_END