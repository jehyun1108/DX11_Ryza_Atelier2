#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL StructuredBoneAllocator
{
public:
	void Init(ID3D11Device* device, ID3D11DeviceContext* context, _uint elementSizeByBytes, _uint capacityElements);

	void BeginFrameWrite();
	void EndFrameWrite();

	void EnsureCapacity(_uint minElements);
	void Destroy();

	_uint Alloc(_uint count);
	void  Write(_uint baseElement, const void* src, _uint count);

	void BindVS(_uint tSlot);
	void BindPS(_uint tSlot);

	ID3D11ShaderResourceView* GetSRV()      const { return srv.Get(); }
	_uint                     GetCapacity() const { return capacity; }

private:
	void CreateResources();
	void MapDiscard();
	void UnmapIfMapped();

private:
	ComPtr<ID3D11Buffer>             buf;
	ComPtr<ID3D11ShaderResourceView> srv;
	ID3D11Device*                    device{};
	ID3D11DeviceContext*             context{};
	_uint                            elementSize{};
	_uint                            capacity{};
	_uint                            cursor{};
	uint8_t*                         mapped{};
	bool                             isMapped{};
};

NS_END