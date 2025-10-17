#include "Enginepch.h"

AssetSystem::AssetSystem()
	:textureCache(textureRegistry, [](const wstring& normalizedKey, const TextureMeta& meta) -> shared_ptr<Texture>
		{ 
			return Texture::LoadFromFile(meta.fullPath, meta.colorSpace);
		}), 
	 shaderCache(shaderRegistry, [](const wstring& normalizedKey, const ShaderMeta& meta) -> shared_ptr<Shader>
		 { 
			 ShaderCreateDesc desc{};
			 desc.shaderTypes = meta.shaderTypes;
			 desc.layout      = meta.layout;

			 if (meta.shaderTypes & SHADER::VS) 
			 {
				 if (meta.vsCsoPath.empty()) 
				 { 
					 assert(false && "VS required but vsCsoPath empty"); 
					 return nullptr;
				 }
				 if (FAILED(D3DReadFileToBlob(meta.vsCsoPath.c_str(), &desc.vsBlob))) 
				 { 
					 assert(false && "Failed VS blob"); 
					 return nullptr; 
				 }
			 }
			 if (meta.shaderTypes & SHADER::PS)
			 {
				 if (meta.psCsoPath.empty()) 
				 {
					 assert(false && "PS required but psCsoPath empty");
					 return nullptr; 
				 }
				 if (FAILED(D3DReadFileToBlob(meta.psCsoPath.c_str(), &desc.psBlob))) 
				 { 
					 assert(false && "Failed PS blob");
					 return nullptr; 
				 }
			 }
			 return Shader::CreateFromBlobs(desc);
		 }),
	modelCache(modelRegistry, [this](const wstring& normalizedKey, const ModelMeta& meta) -> shared_ptr<Model>
		{
			auto model = Model::LoadFromFile(meta.fullPath);
			if (!model) return nullptr;
			model->SetLogicalKey(normalizedKey);

			if (meta.resolveMaterials)
				model->ResolveMaterials(this->shaderCache, this->textureCache);
			return model;
		})
{
}

// ------------------ Texture --------------------------------------------
void AssetSystem::RegisterTexture(const wstring& key, const TextureMeta& meta)
{
	textureRegistry.Upsert(key, meta);
}

shared_ptr<Texture> AssetSystem::GetTexture(const wstring& key)
{
	return textureCache.Ensure(key);
}

void AssetSystem::DropTexture(const wstring& key)
{
	textureCache.Erase(key);
}

// -------------------- Shader -----------------------------------------
void AssetSystem::RegisterShader(const wstring& key, const ShaderMeta& meta)
{
	shaderRegistry.Upsert(key, meta);
}

shared_ptr<Shader> AssetSystem::GetShader(const wstring& key)
{
	return shaderCache.Ensure(key);
}

void AssetSystem::DropShader(const wstring& key)
{
	shaderCache.Erase(key);
}

// ------------------------- Model ------------------------------------
void AssetSystem::RegisterModel(const wstring& key, const ModelMeta& meta)
{
	modelRegistry.Upsert(key, meta);
}

shared_ptr<Model> AssetSystem::GetModel(const wstring& key)
{
	return modelCache.Ensure(key);
}

void AssetSystem::DropModel(const wstring& key)
{
	modelCache.Erase(key);
}

// --------------------------- Clear ---------------------------------
void AssetSystem::ClearCaches()
{
	textureCache.Clear();
	shaderCache.Clear();
	modelCache.Clear();
}

void AssetSystem::ClearRegistries()
{
	textureRegistry.Clear();
	shaderRegistry.Clear();
	modelRegistry.Clear();
}

void AssetSystem::Init()
{
	textureCache.Prime(L"builtin/white",
		Texture::CreateSolidColor(255, 255, 255, 255, TextureColorSpace::sRGB));
	textureCache.Prime(L"builtin/black",
		Texture::CreateSolidColor(0, 0, 0, 255, TextureColorSpace::sRGB));
	textureCache.Prime(L"builtin/gray",
		Texture::CreateSolidColor(128, 128, 128, 255, TextureColorSpace::sRGB));

	textureCache.Prime(L"builtin/flat_normal",
		Texture::CreateSolidColor(128, 128, 255, 255, TextureColorSpace::Linear));
	textureCache.Prime(L"builtin/one_linear",
		Texture::CreateSolidColor(255, 255, 255, 255, TextureColorSpace::Linear));
	textureCache.Prime(L"builtin/zero_linear",
		Texture::CreateSolidColor(0, 0, 0, 255, TextureColorSpace::Linear));

	{
		ShaderMeta meta{};
		meta.shaderTypes = SHADER::VS | SHADER::PS;
		meta.layout = InputLayoutBuilder::MakePNUTan();
		meta.vsCsoPath = L"../bin/Shaders/PNUTan_VS.cso";
		meta.psCsoPath = L"../bin/Shaders/PNUTan_PS.cso";
		RegisterShader(L"PNUTan", meta);
	}

	{
		ShaderMeta meta{};
		meta.shaderTypes = SHADER::VS | SHADER::PS;
		meta.layout = InputLayoutBuilder::MakePNUTanSkin();
		meta.vsCsoPath = L"../bin/Shaders/PNUTanSkin_VS.cso";
		meta.psCsoPath = L"../bin/Shaders/PNUTanSkin_PS.cso";
		RegisterShader(L"PNUTanSkin", meta);
	}

	{
		ShaderMeta meta{};
		meta.shaderTypes = SHADER::VS | SHADER::PS;
		meta.layout = InputLayoutBuilder::MakeVertexColor();
		meta.vsCsoPath = L"../bin/Shaders/PC_VS.cso";
		meta.psCsoPath = L"../bin/Shaders/PC_PS.cso";
		RegisterShader(L"PC", meta);
	}
}