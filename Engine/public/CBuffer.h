#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL CBuffer 
{
public:
	static shared_ptr<CBuffer> Create(size_t byteSize);

public:
	shared_ptr<CBuffer> Clone() const;

	void SetData(const void* data, size_t byteSize);

	template<typename T>
	void SetData(const T& t) { SetData(&t, sizeof(T)); }

	void Update();
	void Bind(SHADER stage, CBUFFERSLOT slot);

	const void* GetData() const { return cpuData.data(); }
	const D3D11_BUFFER_DESC& GetDesc() const { return desc; }

private:
	explicit CBuffer(size_t byteSize) { Resize(byteSize); }
	HRESULT Init();

	void Resize(size_t byteSize);

private:
	ComPtr<ID3D11Buffer> cBuffer{};
	D3D11_BUFFER_DESC    desc{};
	vector<std::byte>    cpuData{};
	bool isdirty = false;
};

NS_END