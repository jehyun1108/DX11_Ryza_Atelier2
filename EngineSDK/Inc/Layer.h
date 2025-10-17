#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL Layer final
{
public:
	Layer() = default;
	virtual ~Layer() {}
	Layer(const Layer&) = delete;
	Layer& operator=(const Layer&) = delete;

public:
	static unique_ptr<Layer> Create() { return make_unique<Layer>(); }

	void AddObj(unique_ptr<Obj> newObj);
	bool RemoveObj(Obj* obj);

	const vector<unique_ptr<Obj>>& GetObjs() const { return objs; }

	Obj* FindObj(const wstring& name);

private:
	vector<unique_ptr<Obj>> objs;
};

NS_END
