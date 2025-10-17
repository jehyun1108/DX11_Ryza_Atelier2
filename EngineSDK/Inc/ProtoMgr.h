#pragma once

NS_BEGIN(Engine)
class Obj;

class ENGINE_DLL ProtoMgr final
{
public:
	ProtoMgr() = default;
	virtual ~ProtoMgr() = default;
	
	ProtoMgr(const ProtoMgr&) = delete;
	ProtoMgr& operator=(const ProtoMgr&) = delete;

public:
	static unique_ptr<ProtoMgr> Create() { return make_unique<ProtoMgr>(); }
	void AddProto(_uint levelID, const wstring& protoTag, unique_ptr<Obj> protoObj);
	unique_ptr<Obj> CloneObj(_uint levelID, const wstring& protoTag, const any& arg);
	void ClearObj(_uint levelID);

private:
	map<_uint, map<wstring, unique_ptr<Obj>>> protoObjs;
};

NS_END