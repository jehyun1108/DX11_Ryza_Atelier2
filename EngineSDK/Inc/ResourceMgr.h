#pragma once

NS_BEGIN(Engine)
class Resource;
class Shader;
class TerrainMesh;
class Texture;
class Model;
class StaticMesh;
class Material;

struct ShaderInfo
{
	InputLayout layout;
	SHADER shaderTypes = SHADER::NONE;
	wstring vsKey;
	wstring psKey;
};

class ENGINE_DLL ResourceMgr final
{
public:
	static unique_ptr<ResourceMgr> Create();
	void Init();

public:
	shared_ptr<Mesh>        GetMesh(const wstring& key)     { return Get(key, meshes); }
	shared_ptr<Texture>     GetTexture(const wstring& key)  { return Get(key, textures); }
	shared_ptr<Shader>      GetShader(const wstring& key)   { return Get(key, shaders); }
	shared_ptr<Model>       GetModel(const wstring& key)    { return Get(key, models); }

	shared_ptr<Texture>     LoadTexture(const wstring& key, const wstring& fullPath, TextureColorSpace colorSpace);
	shared_ptr<Model>       LoadModel(const wstring& fullPath);
	shared_ptr<Shader>      LoadShader(const wstring& key);

private:
	void CreateShaderDescs();
	void CreatePrimitiveMeshes();

	template<typename T>
	shared_ptr<T> Get(const wstring& key, const map<wstring, shared_ptr<T>>& container)
	{
		auto it = container.find(key);
		if (it != container.end())
			return it->second;

		return nullptr;
	}

private:
	map<wstring, shared_ptr<Mesh>>    meshes;
	map<wstring, shared_ptr<Texture>> textures;
	map<wstring, shared_ptr<Shader>>  shaders;
	map<wstring, shared_ptr<Model>>   models;

	map<wstring, ShaderInfo> shaderDescs;
};

NS_END
