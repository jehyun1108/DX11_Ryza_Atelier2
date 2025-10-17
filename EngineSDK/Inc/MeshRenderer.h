#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL MeshRenderer final : public Component
{
public:
	virtual void CopyFrom(const Component* other) {}
};

NS_END