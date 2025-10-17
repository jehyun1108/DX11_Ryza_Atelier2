#pragma once

NS_BEGIN(Engine)

class VIBuffer final
{
public:
	HRESULT Init(void* _vertices, _uint _vtxCount, _uint _vtxStride, void* _indices,
		_uint _idxCount, DXGI_FORMAT _idxFmt, D3D_PRIMITIVE_TOPOLOGY _topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

public:
	void Bind();
	void Draw();
	
protected:
	GameInstance& game = GameInstance::GetInstance();

	ComPtr<ID3D11Buffer>   vb{};
	ComPtr<ID3D11Buffer>   ib{};

	ID3D11Device*          device{};
	ID3D11DeviceContext*   context{};

	_uint                  vtxCount{};
	_uint                  idxCount{};
	
	_uint                  idxStride{};
	_uint                  vtxStride{};

	DXGI_FORMAT            idxFmt{};
	D3D_PRIMITIVE_TOPOLOGY topology{};
};

NS_END