#include "Enginepch.h"

void GpuUtil::EnsureDynamicBuffer(ID3D11Device* device, ID3D11Buffer** buffer, _uint requiredBytes, _uint& capBytes, D3D11_BIND_FLAG bindFlags, float growFactor)
{
	if (requiredBytes == 0)
	{
		if (*buffer)
		{
			(*buffer)->Release();
			*buffer = nullptr;
		}
		capBytes = 0;
		return;
	}
	if (*buffer && capBytes >= requiredBytes) return;

	const _uint target = max(requiredBytes, (_uint)(capBytes * growFactor));
	const _uint newBytes = Utility::AlignTo16Bytes(target);

	D3D11_BUFFER_DESC desc{};
	desc.ByteWidth = newBytes;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = bindFlags;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	ComPtr<ID3D11Buffer> buf;
	HR(device->CreateBuffer(&desc, nullptr, &buf));
	*buffer = buf.Detach();
	capBytes = newBytes;
}

void GpuUtil::EnsureDynamicBuffer(ID3D11Device* device, ComPtr<ID3D11Buffer>& buffer, _uint requiredBytes, _uint& capBytes, D3D11_BIND_FLAG bindFlag, float growFactor)
{
	ID3D11Buffer* pOld = buffer.Get();
	ID3D11Buffer* raw  = pOld;

	EnsureDynamicBuffer(device, &raw, requiredBytes, capBytes, bindFlag, growFactor);
	
	if (raw != pOld)
	{
		buffer.Reset();
		buffer.Attach(raw);
	}
}

void GpuUtil::UploadDynamicBuffer(ID3D11DeviceContext* context, ID3D11Buffer* buffer, const void* src, _uint bytes)
{
	if (!buffer || bytes == 0) return;
	D3D11_MAPPED_SUBRESOURCE mapped{};
	HR(context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
	memcpy(mapped.pData, src, bytes);
	context->Unmap(buffer, 0);
}