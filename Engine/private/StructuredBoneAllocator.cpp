#include "Enginepch.h"
#include "StructuredBoneAllocator.h"
#include "RenderTargetMinimap.h"

void StructuredBoneAllocator::Init(ID3D11Device* dev, ID3D11DeviceContext* ctx, _uint elementSizeByBytes, _uint capacityElements)
{
	device = dev;
	context = ctx;
	elementSize = elementSizeByBytes;
	capacity = capacityElements;
	CreateResources();
}

void StructuredBoneAllocator::BeginFrameWrite()
{
	cursor = 0;
	MapDiscard();
}

void StructuredBoneAllocator::EndFrameWrite()
{
	UnmapIfMapped();
}

void StructuredBoneAllocator::EnsureCapacity(_uint minElements)
{
	if (minElements <= capacity) return;

	_uint newCap = capacity ? capacity : 1024;
	while (newCap < minElements) newCap <<= 1;

	Destroy();
	capacity = newCap;
	CreateResources();
}

void StructuredBoneAllocator::Destroy()
{
	if (isMapped) UnmapIfMapped();
	srv.Reset();
	buf.Reset();
}

_uint StructuredBoneAllocator::Alloc(_uint count)
{
	assert(count > 0 && cursor + count <= capacity);
	_uint base = cursor;
	cursor += count;
	return base;
}

void StructuredBoneAllocator::Write(_uint baseElement, const void* src, _uint count)
{
	assert(isMapped && mapped && src && count > 0);
	memcpy(mapped + baseElement * elementSize, src, size_t(count) * elementSize);
}

void StructuredBoneAllocator::BindVS(_uint tSlot)
{
	assert(!isMapped);
	ID3D11ShaderResourceView* v = srv.Get();
	context->VSSetShaderResources(tSlot, 1, &v);
}

void StructuredBoneAllocator::BindPS(_uint tSlot)
{
	assert(!isMapped);
	ID3D11ShaderResourceView* v = srv.Get();
	context->PSSetShaderResources(tSlot, 1, &v);
}

void StructuredBoneAllocator::CreateResources()
{
	D3D11_BUFFER_DESC desc{};
	desc.ByteWidth           = elementSize * capacity;
	desc.BindFlags           = D3D11_BIND_SHADER_RESOURCE;
	desc.Usage               = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = elementSize;

	HRESULT hr = device->CreateBuffer(&desc, nullptr, buf.GetAddressOf());
	assert(SUCCEEDED(hr));

	D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
	sd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	sd.Format = DXGI_FORMAT_UNKNOWN;
	sd.Buffer.FirstElement = 0;
	sd.Buffer.NumElements = capacity;

	hr = device->CreateShaderResourceView(buf.Get(), &sd, srv.GetAddressOf());
	assert(SUCCEEDED(hr));
}

void StructuredBoneAllocator::MapDiscard()
{
	assert(!isMapped);
	D3D11_MAPPED_SUBRESOURCE m{};
	HRESULT hr = context->Map(buf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &m);
	assert(SUCCEEDED(hr));
	mapped = static_cast<uint8_t*>(m.pData);
	isMapped = true;
}

void StructuredBoneAllocator::UnmapIfMapped()
{
	if (!isMapped) return;
	context->Unmap(buf.Get(), 0);
	mapped = nullptr;
	isMapped = false;
}