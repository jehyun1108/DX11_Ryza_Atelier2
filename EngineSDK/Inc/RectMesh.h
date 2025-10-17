#pragma once

NS_BEGIN(Engine)

class RectMesh final : public Mesh
{
public:
	static shared_ptr<RectMesh> Create(ID3D11Device* device, bool keepCPU = false);
	virtual HRESULT Init(ID3D11Device* device, bool keepCPU = false);
};

NS_END