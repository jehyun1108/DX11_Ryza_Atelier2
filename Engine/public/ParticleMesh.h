#pragma once

#include "ParticleMeshData.h"

NS_BEGIN(Engine)

class ENGINE_DLL ParticleMesh
{
public:
	void Create(ID3D11Device* device, int maxInstances);
	void Draw(ID3D11DeviceContext* ctx, const vector<const ParticleDrawItem*>& items);

private:
	ComPtr<ID3D11Buffer> quadVB;
	ComPtr<ID3D11Buffer> quadIB;
	ComPtr<ID3D11Buffer> instanceVB;
	
	int maxInstances = 0;
};

NS_END