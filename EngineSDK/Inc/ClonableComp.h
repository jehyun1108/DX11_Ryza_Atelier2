#pragma once

NS_BEGIN(Engine)

template<typename T>
class ClonableComp : public Component
{
public:
	using Component::Component; // 부모 생성자 그대로 사용

	virtual unique_ptr<Component> Clone(Obj* newOwner) const override
	{
		const T& self = static_cast<const T&>(*this);
		auto newComp = make_unique<T>(self);
		newComp->Init();
		newComp->owner = newOwner;
		return newComp;
	}
};


NS_END