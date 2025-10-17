#pragma once

NS_BEGIN(Engine)

class Mesh_Rect final : public Mesh
{
public:
	static shared_ptr<Mesh_Rect> Create();

	HRESULT Init();
};

NS_END