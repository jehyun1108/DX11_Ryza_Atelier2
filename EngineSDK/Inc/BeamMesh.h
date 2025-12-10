#pragma once

#include "BeamMeshData.h"

NS_BEGIN(Engine)

class ENGINE_DLL BeamMesh
{
public:
    void Create(ID3D11Device* device, int maxVertices, int maxIndices);
    void Draw(ID3D11DeviceContext* ctx, const BeamSnapshot& snapshot, const CameraProxy& cam);

private:
    ComPtr<ID3D11Buffer> vb;
    ComPtr<ID3D11Buffer> ib;
    int maxVertices{};
    int maxIndices{};
};

NS_END