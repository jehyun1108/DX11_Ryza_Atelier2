#pragma once

NS_BEGIN(Engine)
class TransformSystem;
class GameInstance;
class RenderScene;
class Layer;
class Obj;

class ENGINE_DLL ObjMgr final
{
public:
	ObjMgr(_uint _levelCount, SystemRegistry& registry);
	~ObjMgr() {}
	ObjMgr(const ObjMgr&) = delete;
	ObjMgr& operator=(const ObjMgr&) = delete;

public:
	static unique_ptr<ObjMgr> Create(_uint levelCount, SystemRegistry& registry) { return make_unique<ObjMgr>(levelCount, registry); }

	template<typename T>
	T* AddObj(const ObjDesc& desc)
	{
		unique_ptr<T> obj = make_unique<T>();
		Obj* pObj = obj.get();

		EntityID id = static_cast<EntityID>(++nextID);
		pObj->SetID(id);
		pObj->SetName(desc.name);

		if (desc.tfDesc)
		{
			auto& tf = registry.Get<TransformSystem>();
			Handle handle = tf.Create(id, desc.tfDesc.value());
			pObj->Handles().transform = handle;
		}

		GetLayer(desc.levelID, desc.layerType)->AddObj(move(obj));
		return static_cast<T*>(pObj);
	}

	void DestroyObj(Obj* obj);

	Layer* GetLayer(LAYER type);
	vector<unique_ptr<Layer>>* GetLayers();
	const vector<unique_ptr<Layer>>* GetLayers() const;

	Obj* FindObj(const wstring& name, LAYER type);
	Obj* FindObj(const wstring& name);
	void Clear();

	vector<Obj*> GetObjs();
	vector<Obj*> GetObjs(LAYER type);

private:
	Layer* GetLayer(_uint levelID, LAYER type);

private:
	_uint curLevelID = 0;
	_uint levelCount{};
	vector<vector<unique_ptr<Layer>>> layers;
	
	SystemRegistry& registry;
	_uint nextID = 0;
};

NS_END