#include "Enginepch.h"
#include "RingCBAllocator.h"
#include "StructuredBoneAllocator.h"

void RingCBAllocator::Init(ID3D11Device* dev, ID3D11DeviceContext* ctx, _uint totalConstants)
{
	device = dev;
	context = ctx;
	QueryContext1();

	capacity = totalConstants;
	ZeroMemory(&desc, sizeof(desc));
	desc.ByteWidth      = capacity * 16;
	desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
	desc.Usage          = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HR(device->CreateBuffer(&desc, nullptr, cb.GetAddressOf()));
}

void RingCBAllocator::BeginFrameWrite()
{
	cursor = 0;
	MapDiscard();
}

void RingCBAllocator::EndFrameWrite()
{
	UnmapIfMapped();
}

_uint RingCBAllocator::Alloc(_uint constantsNeeded)
{
	assert(constantsNeeded > 0);
	if (cursor + constantsNeeded > capacity)
	{
		EndFrameWrite();
		cursor = 0;
		MapDiscard();
	}

	UINT first = cursor;
	cursor += constantsNeeded;
	return first;
}

void RingCBAllocator::Write(_uint firstConstant, const void* src, size_t bytes)
{
	assert(mapped && mappedPtr);
	assert(src && bytes > 0);
	memcpy(mappedPtr + firstConstant * 16, src, bytes);
}

void RingCBAllocator::BindRange(SHADER stage, CBUFFERSLOT slot, _uint firstConstant, _uint numConstants)
{
	assert(!mapped && "Unmap before binding");
	ID3D11Buffer* buf = cb.Get();
	UINT first = firstConstant;
	UINT count = numConstants;
	const UINT idx = ENUM(slot);

	if (stage & SHADER::VS) context1->VSSetConstantBuffers1(idx, 1, &buf, &first, &count);
	if (stage & SHADER::PS) context1->PSSetConstantBuffers1(idx, 1, &buf, &first, &count);
	//if (stage & SHADER::HS) context1->HSSetConstantBuffers1(idx, 1, &buf, &first, &count);
	//if (stage & SHADER::DS) context1->DSSetConstantBuffers1(idx, 1, &buf, &first, &count);

	assert(ENUM(CBUFFERSLOT::OBJ) < D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);
}

void RingCBAllocator::MapDiscard()
{
	assert(!mapped && "already mapped");
	D3D11_MAPPED_SUBRESOURCE m{};
	HRESULT hr = context->Map(cb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &m);
	assert(SUCCEEDED(hr));
	mappedPtr = static_cast<uint8_t*>(m.pData);
	mapped = true;
}

void RingCBAllocator::UnmapIfMapped()
{
	if (!mapped) return;
	context->Unmap(cb.Get(), 0);
	mappedPtr = nullptr;
	mapped = false;
}

void RingCBAllocator::QueryContext1()
{
	if (context1) return;
	HRESULT hr = context->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&context1));
	assert(SUCCEEDED(hr) && context1);
}