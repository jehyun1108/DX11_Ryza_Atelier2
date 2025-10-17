#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL EngineCore final : public Base, public Singleton<EngineCore>
{
public:
	EngineCore(PassKey) {}
	virtual ~EngineCore() = default;

#pragma region Engine
public:
	HRESULT Init(const EngineDesc& engineDesc);
#pragma endregion

#pragma region TimeMgr
public:
	_float GetDt(const wstring& tag);
	HRESULT AddTimer(const wstring& tag);
	void UpdateDt(const wstring& tag);
#pragma endregion

private:
	class TimeMgr* timeMgr{};
	
public:
	virtual void Free() override;
};

NS_END