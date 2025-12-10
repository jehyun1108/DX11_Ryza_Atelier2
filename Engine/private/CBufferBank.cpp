#include "Enginepch.h"

void CBufferBank::Init(ID3D11Device* dev, ID3D11DeviceContext* ctx, const array<_uint, CBufferCount>& size)
{
	device  = dev;
	context = ctx;

	for (size_t i = 0; i < CBufferCount; ++i)
	{
		assert(size[i] > 0);
		buffers[i] = CBuffer::Create(size[i]);
		assert(buffers[i]);
	}
}

void CBufferBank::UpdateRaw(CBufferID id, const void* p, _uint bytes)
{
	assert(p && bytes > 0);
	auto& b = buffers[ENUM(id)];
	assert(b);
	b->SetData(p, bytes);
	b->Update();
}

void CBufferBank::Bind(CBufferID id, SHADER stages, CBUFFERSLOT slot)
{
	auto& b = buffers[ENUM(id)];
	assert(b);
	b->Bind(stages, slot);
}

CBuffer* CBufferBank::Get(CBufferID id) const
{
	auto& b = buffers[ENUM(id)];
	assert(b);
	return b.get();
}