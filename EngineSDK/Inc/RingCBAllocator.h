#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL RingCBAllocator
{
public:
	void Init(ID3D11Device* device, ID3D11DeviceContext* context, _uint totalConstants = 4096);

	void BeginFrameWrite();
	void EndFrameWrite();

	_uint Alloc(_uint constantsNeeded);
	void  Write(_uint firstConstant, const void* src, size_t bytes);
	void  BindRange(SHADER stage, CBUFFERSLOT slot, _uint firstConstant, _uint numConstants);

	ID3D11DeviceContext* GetContext() const { return context; }
	ID3D11DeviceContext1* GetContext1() const { return context1; }

	ID3D11Buffer* GetBuffer()            const { return cb.Get(); }
	_uint         GetCapacityConstants() const { return capacity; }

private:
	void MapDiscard();
	void UnmapIfMapped();
	void QueryContext1();

private:
	ComPtr<ID3D11Buffer>  cb{};
	D3D11_BUFFER_DESC     desc{};
	ID3D11Device*         device{};
	ID3D11DeviceContext*  context{};
	ID3D11DeviceContext1* context1{};

	_uint    capacity  = 0;
	_uint    cursor    = 0;
	uint8_t* mappedPtr = nullptr;
	bool     mapped    = false;
};

NS_END