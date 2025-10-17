#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL StaticMesh : public Mesh
{
public:
	virtual HRESULT Init(ID3D11Device* device, const MeshDesc& desc) override;
};

NS_END