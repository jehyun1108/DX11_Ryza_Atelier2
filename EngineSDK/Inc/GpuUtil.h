#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL GpuUtil
{
public:
	static void EnsureDynamicBuffer(ID3D11Device* device, ID3D11Buffer** buffer, _uint requiredBytes, _uint& capBytes, D3D11_BIND_FLAG bindFlags = D3D11_BIND_VERTEX_BUFFER, float growFactor = 1.5f);
	static void EnsureDynamicBuffer(ID3D11Device* device, ComPtr<ID3D11Buffer>& buffer, _uint requiredBytes, _uint& capBytes, D3D11_BIND_FLAG bindFlag = D3D11_BIND_VERTEX_BUFFER, float growFactor = 1.5f);
	static void UploadDynamicBuffer(ID3D11DeviceContext* context, ID3D11Buffer* buffer, const void* src, _uint bytes);
};

NS_END