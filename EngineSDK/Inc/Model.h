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

	Skeleton*                           GetSkeletonRaw()      const { return skeleton.get(); }
	shared_ptr<Skeleton>                GetSkeleton()         const { return skeleton; }
	const ClipTable*                    GetClipTable()        const { return &clipTable; }
	const vector<ModelParts>&           GetParts()            const { return parts; }
	const vector<shared_ptr<AnimClip>>& GetAnimClips()        const { return animClips; }
	const vector<shared_ptr<Material>>& GetMaterials()        const { return materials; }
	const vector<_float4x4>&            GetBindPoseMatrices() const { return bindPoseMatrices; }
	
	const Mesh* GetFirstMesh() const;
	const Mesh* GetLargestMeshByAABB() const;
	void        GetAllMeshes(vector<const Mesh*>& out) const;

	const BoundingBox& GetBoundingBox()  const { return boundingBox; }
	bool               IsSkeletalModel() const { return skeleton != nullptr; }

	void ResolveMaterials(ShaderCache& shaderCache, TextureCache& textureCache);
	void SetLogicalKey(wstring key)  { logicalKey = move(key); }
	const wstring& GetLogicalKey() const { return logicalKey; }

private:
	HRESULT InitFromFile(const wstring& fullPath);
	void    ReadMaterials(ifstream& inFile, const filesystem::path& modelDir);
	void    ReadMeshes(ifstream& inFile);
	void    ReadSkeletons(ifstream& inFile);
	void    ReadAnimations(ifstream& inFile);

	void    TryNormalFromDiffuseMap(Material& targetMaterial, const wstring& diffuseFullPath, const wstring& baseKey);
	void    BuildBindPose(const Skeleton& skeleton, vector<_float4x4>& out);
	void    FinalSetUp();

	bool    IsSpecialCharacter(const wstring& key) { return key == L"ryza" || key == L"patricia" || key == L"klaudia"; }
	void    ApplyDefaultSkinMapping(const wstring& baseKey, const wstring& diffuseStem, Material& material, const filesystem::path& dir);
	void    ApplySpecialMapping(const wstring& baseKey, const wstring& diffuseStem, Material& material, const filesystem::path& dir);

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
	vector<_float4x4> bindPoseMatrices;

	bool isSkinned = false;

	ClipTable clipTable;
};

NS_END