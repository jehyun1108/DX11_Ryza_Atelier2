#pragma once

NS_BEGIN(Engine)
class Texture;
class Shader;
class Model;

using TextureRegistry = AssetRegistry<TextureMeta>;
using ShaderRegistry  = AssetRegistry<ShaderMeta>;
using ModelRegistry   = AssetRegistry<ModelMeta>;
//using MeshRegistry = AssetRegistry<MeshMeta>;

using TextureCache    = AssetCache<shared_ptr<Texture>, TextureMeta, TextureRegistry>;
using ShaderCache     = AssetCache<shared_ptr<Shader>,  ShaderMeta, ShaderRegistry>;
using ModelCache      = AssetCache<shared_ptr<Model>,   ModelMeta, ModelRegistry>;

class ENGINE_DLL AssetSystem
{
public:
	AssetSystem(const AssetSystem&) = delete;
	AssetSystem& operator=(const AssetSystem&) = delete;
	AssetSystem(AssetSystem&&) = delete;
	AssetSystem& operator=(AssetSystem&&) = delete;

public:
	explicit AssetSystem();
	void Init();

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

	// ---------- Util --------------------------------
	void ClearCaches();
	void ClearRegistries();

private:
	TextureRegistry textureRegistry;
	ShaderRegistry  shaderRegistry;
	ModelRegistry   modelRegistry;

	TextureCache    textureCache;
	ShaderCache     shaderCache;
	ModelCache      modelCache;
};

NS_END