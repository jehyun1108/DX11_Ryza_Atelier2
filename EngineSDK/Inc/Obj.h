#pragma once

#include "ObjHandles.h"

NS_BEGIN(Engine)

class ENGINE_DLL Obj final
{
public:
	EntityID GetID() const { return id; }
	void     SetID(EntityID v) { id = v; }

	void           SetName(wstring name) { this->name = move(name); }
	const wstring& GetName() const { return name; }

	ObjHandles&       Handles() { return handles; }
	const ObjHandles& Handles() const { return handles; }

	void SetTransformHandle(Handle handle) { handles.transform = handle; }

private:
	EntityID   id = invalidEntity;
	wstring    name{};
	ObjHandles handles{};
};

NS_END