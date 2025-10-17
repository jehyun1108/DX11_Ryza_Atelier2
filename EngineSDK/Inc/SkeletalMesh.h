#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL SkeletalMesh final : public Mesh
{ 
public:
	virtual HRESULT Init(ID3D11Device* device, const MeshDesc& desc) override;

	void SetSkeleton(shared_ptr<Skeleton> skeleton) { this->skeleton = move(skeleton); }
	shared_ptr<Skeleton> GetSkeleton() { return skeleton; }

	void SetType(MESHTYPE _type) { type = _type; }
	MESHTYPE GetType() const { return type; }

	void UpdateBoneBuffer(ID3D11DeviceContext* context, BoneMatrices boneMatrices);
	void BindBoneBuffer(ID3D11DeviceContext* context);

private:
	shared_ptr<Skeleton> skeleton;
	ComPtr<ID3D11Buffer> boneBuffer{};
	MESHTYPE type = MESHTYPE::ANIMATED;
};

NS_END