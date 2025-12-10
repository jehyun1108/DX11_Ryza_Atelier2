#pragma once

NS_BEGIN(Engine)
class Texture;
class Shader;
class Model;
class Mesh;
class Material;

using TextureRegistry  = AssetRegistry<TextureMeta>;
using ShaderRegistry   = AssetRegistry<ShaderMeta>;
using ModelRegistry    = AssetRegistry<ModelMeta>;
using MeshRegistry     = AssetRegistry<MeshMeta>;
using MaterialRegistry = AssetRegistry<MaterialMeta>;

using TextureCache    = AssetCache<shared_ptr<Texture>,  TextureMeta,  TextureRegistry>;
using ShaderCache     = AssetCache<shared_ptr<Shader>,   ShaderMeta,   ShaderRegistry>;
using ModelCache      = AssetCache<shared_ptr<Model>,    ModelMeta,    ModelRegistry>;
using MeshCache       = AssetCache<shared_ptr<Mesh>,     MeshMeta,     MeshRegistry>;
using MaterialCache   = AssetCache<shared_ptr<Material>, MaterialMeta, MaterialRegistry>;

class ENGINE_DLL AssetSystem : public ISystem
{
public:
	explicit AssetSystem(SystemRegistry& registry);
	void     OnBoot() override;
	void     Init();

	// ---------  Texture ----------------------
	void RegisterTexture(const wstring& key, const TextureMeta& meta);
	shared_ptr<Texture> GetTexture(const wstring& key);
	void DropTexture(const wstring& key);

	// --------- Shader --------------------------
	void RegisterShader(const wstring& key, const ShaderMeta& meta);
	shared_ptr<Shader> GetShader(const wstring& key);
	void DropShader(const wstring& key);

	// ----------- Model ------------------------------
	void RegisterModel(const wstring& key, const ModelMeta& meta);
	shared_ptr<Model> GetModel(const wstring& key);
	void DropModel(const wstring& key);

	// ---------- Mesh ----------
	void RegisterMesh(const wstring& key, const MeshMeta& meta);
	shared_ptr<Mesh> GetMesh(const wstring& key);
	void DropMesh(const wstring& key);

	// ---------- Material ----------
	void RegisterMaterial(const wstring& key, const MaterialMeta& desc);
	shared_ptr<Material> GetMaterial(const wstring& key);
	void DropMaterial(const wstring& key);

	// ---------- Util --------------------------------
	void ClearCaches();
	void ClearRegistries();

	void EnsureEffectTextures(const EffectArchetype& effect);

	MaterialCache& GetMaterialCache() { return materialCache; }
	TextureCache&  GetTextureCache()  { return textureCache; }
	ShaderCache&   GetShaderCache()   { return shaderCache; }

private:
	SystemRegistry&  registry;

	TextureRegistry  textureRegistry;
	ShaderRegistry   shaderRegistry;
	ModelRegistry    modelRegistry;
	MeshRegistry     meshRegistry;
	MaterialRegistry materialRegistry;

	TextureCache     textureCache;
	ShaderCache      shaderCache;
	ModelCache       modelCache;
	MeshCache        meshCache;
	MaterialCache    materialCache;

	mutable recursive_mutex mtx;
};

NS_END