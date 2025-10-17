#include "Enginepch.h"

shared_ptr<CBuffer> CBuffer::Create(size_t byteSize)
{
	auto cb = shared_ptr<CBuffer>(new CBuffer(byteSize));
	if (FAILED(cb->Init())) 
		return nullptr;
	return cb;
}

HRESULT CBuffer::Init()
{
	HR(DEVICE->CreateBuffer(&desc, nullptr, cBuffer.GetAddressOf()));
	return S_OK;
}

shared_ptr<CBuffer> CBuffer::Clone() const
{
	auto clone = shared_ptr<CBuffer>(new CBuffer(desc.ByteWidth));
	if (FAILED(clone->Init())) return nullptr;   
	if (!cpuData.empty())
		memcpy(clone->cpuData.data(), cpuData.data(), cpuData.size());
	clone->isdirty = true;                               
	return clone;
}

void CBuffer::SetData(const void* data, size_t byteSize)
{
	assert(data);
	assert(byteSize <= cpuData.size());
	memset(cpuData.data(), 0, cpuData.size());
	memcpy(cpuData.data(), data, byteSize);
	isdirty = true;
}

void CBuffer::Update()
{
	assert(cBuffer && "CBuffer not initialized");
	if (!isdirty) return;

	D3D11_MAPPED_SUBRESOURCE mapped{};
	HR(DC->Map(cBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
	memcpy(mapped.pData, cpuData.data(), cpuData.size());
	DC->Unmap(cBuffer.Get(), 0);
	isdirty = false;
}

void CBuffer::Bind(SHADER stage, CBUFFERSLOT slot)
{
	assert(cBuffer && "CBuffer not initialized");
	if (isdirty) Update();                 
	ID3D11Buffer* cb = cBuffer.Get();
	const _uint idx = ENUM(slot);

	if (stage & SHADER::VS) DC->VSSetConstantBuffers(idx, 1, &cb);
	if (stage & SHADER::PS) DC->PSSetConstantBuffers(idx, 1, &cb);
	if (stage & SHADER::GS) DC->GSSetConstantBuffers(idx, 1, &cb);
	if (stage & SHADER::HS) DC->HSSetConstantBuffers(idx, 1, &cb);
	if (stage & SHADER::DS) DC->DSSetConstantBuffers(idx, 1, &cb);
	if (stage & SHADER::CS) DC->CSSetConstantBuffers(idx, 1, &cb);
}

void CBuffer::Resize(size_t byteSize)
{
	const _uint aligned  = Utility::AlignTo16Bytes((_uint)byteSize);
	assert(aligned <= 65536u && "cBuffer must be <= 64 KB");
	cpuData.assign(aligned, std::byte{ 0 });
	ZeroMemory(&desc, sizeof(desc));
	desc.ByteWidth      = aligned;
	desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
	desc.Usage          = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	isdirty             = true;
}
