#pragma once

NS_BEGIN(Engine)

struct ModelParts
{
	shared_ptr<Mesh>     mesh;
	shared_ptr<Material> material;
};

class ENGINE_DLL Model final 
{
public:
	Model();
	static shared_ptr<Model>            LoadFromFile(const wstring& fullPath);

	Skeleton*                           GetSkeletonRaw()  const { return skeleton.get(); }
	shared_ptr<Skeleton>                GetSkeleton()     const { return skeleton; }
	const ClipTable*                    GetClipTable()    const { return &clipTable; }
	const vector<ModelParts>&           GetParts()        const { return parts; }
	const vector<shared_ptr<AnimClip>>& GetAnimClips()    const { return animClips; }
	const vector<shared_ptr<Material>>& GetMaterials()    const { return materials; }

	const BoundingBox& GetBoundingBox()  const { return boundingBox; }
	bool               IsSkeletalModel() const { return skeleton != nullptr; }

	void ResolveMaterials(ShaderCache& shaderCache, TextureCache& textureCache);
	void SetLogicalKey(wstring key) { logicalKey = move(key); }
	const wstring& GetLogicalKey() const { return logicalKey; }

private:
	HRESULT InitFromFile(const wstring& fullPath);
	void    ReadMaterials(ifstream& inFile, const filesystem::path& modelDir);
	void    ReadMeshes(ifstream& inFile);
	void    ReadSkeletons(ifstream& inFile);
	void    ReadAnimations(ifstream& inFile);

	void    FinalSetUp();

private:
	GameInstance& game = GameInstance::GetInstance();
	ID3D11Device* device{};
	ID3D11DeviceContext* context{};

	wstring logicalKey;
	vector<ModelParts> parts;
	BoundingBox boundingBox;

	shared_ptr<Skeleton> skeleton;
	vector<shared_ptr<AnimClip>> animClips;
	vector<shared_ptr<Material>> materials;

	bool isSkeletalModel = false;

	ClipTable clipTable;
};

NS_END