#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL ObjBuilder final
{
public:
	explicit ObjBuilder(unique_ptr<Obj> emptyObj, const ObjDesc& blueprint);
	virtual ~ObjBuilder() = default;

public:
	unique_ptr<Obj> Build();

private:
	void BuildNameAndActive();
	void BuildTransform();
	void BuildMesh();

private:
	GameInstance& game = GameInstance::Get();
	const ObjDesc& blueprint;
	unique_ptr<Obj> obj; // 조립 중인 객체
};

NS_END