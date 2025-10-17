#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL Resource abstract 
{
public:
	Resource();
	virtual ~Resource() = default;

	const wstring& GetPath() const { return filePath; }
	const wstring& GetKey() const { return key; }
	void SetKey(const wstring& key) { this->key = key; }

protected:
	wstring key;
	GameInstance& game = GameInstance::GetInstance();
	ID3D11DeviceContext* context{};
	ID3D11Device* device{};
	wstring filePath;
};

NS_END