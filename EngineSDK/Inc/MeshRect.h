#pragma once

NS_BEGIN(Engine)

class MeshRect final : public Mesh
{
public:;
	MeshRect() = default;
	virtual ~MeshRect() = default;

public:
	static shared_ptr<MeshRect> Create();

	HRESULT Init();
};

NS_END