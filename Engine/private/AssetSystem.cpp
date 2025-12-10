#include "Enginepch.h"

AssetSystem::AssetSystem(SystemRegistry& registry) 
	: registry(registry),
	textureCache(textureRegistry, [](const wstring& normalizedKey, const TextureMeta& meta) -> shared_ptr<Texture>
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
			 if (meta.shaderTypes & SHADER::HS)
			 {
				 if (meta.hsCsoPath.empty())
				 {
					 assert(false && "HS required but hsCsoPath empty");
					 return nullptr;
				 }
				 if (FAILED(D3DReadFileToBlob(meta.hsCsoPath.c_str(), &desc.hsBlob)))
				 {
					 assert(false && "Failed HS blob");
					 return nullptr;
				 }
			 }
			 if (meta.shaderTypes & SHADER::DS)
			 {
				 if (meta.dsCsoPath.empty())
				 {
					 assert(false && "DS required but dsCsoPath empty");
					 return nullptr;
				 }
				 if (FAILED(D3DReadFileToBlob(meta.dsCsoPath.c_str(), &desc.dsBlob)))
				 {
					 assert(false && "Failed DS blob");
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
		}),
	meshCache(meshRegistry, [this](const wstring& normalizedKey, const MeshMeta& meta) -> shared_ptr<Mesh> 
		{
			auto mesh = make_shared<Mesh>();

			if (meta.meshKind == MESH::Primitive && meta.primitive != PRIMITIVE::None)
			{
				if (FAILED(mesh->InitPrimitive(DEVICE, meta))) 
					return nullptr;
				return mesh;
			}
			return {};
		}),
		materialCache(materialRegistry, [this](const wstring& normalizedKey, const MaterialMeta& meta) -> shared_ptr<Material> 
		{
				auto material = make_shared<Material>();
				material->Resolve(this->shaderCache, this->textureCache);
				return material;
		})
{
}

void AssetSystem::OnBoot()
{
	Init();
}

// ------------------ Texture --------------------------------------------
void AssetSystem::RegisterTexture(const wstring& key, const TextureMeta& meta)
{
	lock_guard<recursive_mutex> lock(mtx);
	textureRegistry.Upsert(key, meta);
}

shared_ptr<Texture> AssetSystem::GetTexture(const wstring& key)
{
	lock_guard<recursive_mutex> lock(mtx);
	return textureCache.Ensure(key);
}

void AssetSystem::DropTexture(const wstring& key)
{
	lock_guard<recursive_mutex> lock(mtx);
	textureCache.Erase(key);
}

// -------------------- Shader -----------------------------------------
void AssetSystem::RegisterShader(const wstring& key, const ShaderMeta& meta)
{
	lock_guard<recursive_mutex> lock(mtx);
	shaderRegistry.Upsert(key, meta);
}

shared_ptr<Shader> AssetSystem::GetShader(const wstring& key)
{
	lock_guard<recursive_mutex> lock(mtx);
	return shaderCache.Ensure(key);
}

void AssetSystem::DropShader(const wstring& key)
{
	lock_guard<recursive_mutex> lock(mtx);
	shaderCache.Erase(key);
}

// ------------------------- Model ------------------------------------
void AssetSystem::RegisterModel(const wstring& key, const ModelMeta& meta)
{
	lock_guard<recursive_mutex> lock(mtx);
	modelRegistry.Upsert(key, meta);
}

shared_ptr<Model> AssetSystem::GetModel(const wstring& key)
{
	lock_guard<recursive_mutex> lock(mtx);
	return modelCache.Ensure(key);
}

void AssetSystem::DropModel(const wstring& key)
{
	lock_guard<recursive_mutex> lock(mtx);
	modelCache.Erase(key);
}

void AssetSystem::RegisterMesh(const wstring& key, const MeshMeta& meta)
{
	lock_guard<recursive_mutex> lock(mtx);
	meshRegistry.Upsert(key, meta);
}

shared_ptr<Mesh> AssetSystem::GetMesh(const wstring& key)
{
	lock_guard<recursive_mutex> lock(mtx);
	return meshCache.Ensure(key);
}

void AssetSystem::DropMesh(const wstring& key)
{
	lock_guard<recursive_mutex> lock(mtx);
	meshCache.Erase(key);
}

void AssetSystem::RegisterMaterial(const wstring& key, const MaterialMeta& desc)
{
	lock_guard<recursive_mutex> lock(mtx);
	materialRegistry.Upsert(key, desc);
}

shared_ptr<Material> AssetSystem::GetMaterial(const wstring& key)
{
	lock_guard<recursive_mutex> lock(mtx);
	return materialCache.Ensure(key);
}

void AssetSystem::DropMaterial(const wstring& key)
{
	lock_guard<recursive_mutex> lock(mtx);
	materialCache.Erase(key);
}

// --------------------------- Clear ---------------------------------
void AssetSystem::ClearCaches()
{
	lock_guard lock(mtx);
	textureCache.Clear();
	shaderCache.Clear();
	modelCache.Clear();
	meshCache.Clear();
	materialCache.Clear();
}

void AssetSystem::ClearRegistries()
{
	lock_guard lock(mtx);
	textureRegistry.Clear();
	shaderRegistry.Clear();
	modelRegistry.Clear();
	meshRegistry.Clear();
	materialRegistry.Clear();
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
		meta.layout      = InputLayoutBuilder::MakePNUTan();
		meta.vsCsoPath   = L"../bin/Shaders/PNUTan_VS.cso";
		meta.psCsoPath   = L"../bin/Shaders/PNUTan_PS.cso";
		RegisterShader(L"PNUTan", meta);
	}
	{
		ShaderMeta meta{};
		meta.shaderTypes = SHADER::VS | SHADER::PS;
		meta.layout      = InputLayoutBuilder::MakePNUTanSkin();
		meta.vsCsoPath   = L"../bin/Shaders/PNUTanSkin_VS.cso";
		meta.psCsoPath   = L"../bin/Shaders/PNUTanSkin_PS.cso";
		RegisterShader(L"PNUTanSkin", meta);
	}
	//{
	//	ShaderMeta meta{};
	//	meta.shaderTypes = SHADER::VS | SHADER::HS | SHADER::DS | SHADER::PS;
	//	meta.layout      = InputLayoutBuilder::MakePNUTanSkin();
	//	meta.vsCsoPath   = L"../bin/Shaders/PNUTanSkin_VS_TS.cso";
	//	meta.hsCsoPath   = L"../bin/Shaders/PNUTanSkin_HS.cso";
	//	meta.dsCsoPath   = L"../bin/Shaders/PNUTanSkin_DS.cso";
	//	meta.psCsoPath   = L"../bin/Shaders/PNUTanSkin_PS.cso";
	//	RegisterShader(L"PNUTanSkin_TS", meta);
	//}
	{
		ShaderMeta meta{};
		meta.shaderTypes = SHADER::VS | SHADER::PS;
		meta.layout      = InputLayoutBuilder::MakePC();
		meta.vsCsoPath   = L"../bin/Shaders/PC_VS.cso";
		meta.psCsoPath   = L"../bin/Shaders/PC_PS.cso";
		RegisterShader(L"PC", meta);
	}
	{
		ShaderMeta meta{};
		meta.shaderTypes = SHADER::VS | SHADER::PS;
		meta.layout      = InputLayoutBuilder::MakePNUTan();
		meta.vsCsoPath   = L"../bin/Shaders/Skybox_VS.cso";
		meta.psCsoPath   = L"../bin/Shaders/Skybox_PS.cso";
		RegisterShader(L"Skybox", meta);
	}
	{
		ShaderMeta meta{};
		meta.shaderTypes = SHADER::VS | SHADER::PS;
		meta.layout      = InputLayoutBuilder::MakeUI();
		meta.vsCsoPath   = L"../bin/Shaders/PUC_VS_UI.cso";
		meta.psCsoPath   = L"../bin/Shaders/PUC_PS_UI.cso";
		RegisterShader(L"UI", meta);
	}
	{
		ShaderMeta meta{};
		meta.shaderTypes = SHADER::VS | SHADER::PS;
		meta.layout = InputLayoutBuilder::MakePU();
		meta.vsCsoPath = L"../bin/Shaders/Deferred_Directional_VS.cso";
		meta.psCsoPath = L"../bin/Shaders/Deferred_Directional_PS.cso";
		RegisterShader(L"Deferred_Directional", meta);
	}
	{
		ShaderMeta meta{};
		meta.shaderTypes = SHADER::VS | SHADER::PS;
		meta.layout = InputLayoutBuilder::MakePU();
		meta.vsCsoPath = L"../bin/Shaders/Deferred_Composite_VS.cso";
		meta.psCsoPath = L"../bin/Shaders/Deferred_Composite_PS.cso";
		RegisterShader(L"Deferred_Composite", meta);
	}
	{
		ShaderMeta meta{};
		meta.shaderTypes = SHADER::VS | SHADER::PS;
		meta.layout = {};
		meta.vsCsoPath = L"../bin/Shaders/Debug_VS.cso";
		meta.psCsoPath = L"../bin/Shaders/Debug_PS.cso";
		RegisterShader(L"Debug", meta);
	}
	{
		ShaderMeta meta{};
		meta.shaderTypes = SHADER::VS | SHADER::PS;
		meta.layout = InputLayoutBuilder::MakeUI();
		meta.vsCsoPath = L"../bin/Shaders/PUC_VS_UI.cso";
		meta.psCsoPath = L"../bin/Shaders/Minimap_PS.cso";
		RegisterShader(L"field_minimap", meta);
	}
	{
		ShaderMeta meta{};
		meta.shaderTypes = SHADER::VS | SHADER::PS;
		meta.layout = InputLayoutBuilder::MakeParticle();
		meta.vsCsoPath = L"../bin/Shaders/Particle_VS.cso";
		meta.psCsoPath = L"../bin/Shaders/Particle_PS.cso";
		RegisterShader(L"particle", meta);
	}
	{
		ShaderMeta meta{};
		meta.shaderTypes = SHADER::VS | SHADER::PS;
		meta.layout = InputLayoutBuilder::MakeTrail();
		meta.vsCsoPath = L"../bin/Shaders/Trail_VS.cso";
		meta.psCsoPath = L"../bin/Shaders/Trail_PS.cso";
		RegisterShader(L"trail", meta);
	}
	{
		ShaderMeta meta{};
		meta.shaderTypes = SHADER::VS | SHADER::PS;
		meta.layout = InputLayoutBuilder::MakePNUTanSkin();
		meta.vsCsoPath = L"../bin/Shaders/PNUTanSkin_VS.cso";
		meta.psCsoPath = L"../bin/Shaders/Dressing_PS.cso";
		RegisterShader(L"Dressing", meta);
	}
}

void AssetSystem::EnsureEffectTextures(const EffectArchetype& effect)
{
	const filesystem::path baseDir = L"../bin/Resources/Particles/asset/";
	auto loadTexIfNeeded = [&](const wstring& texKey)
		{
			if (texKey.empty()) return;
			if (GetTexture(texKey)) return;

			const wchar_t* exts[] = { L".dds", L".png" };

			for (const auto* ext : exts)
			{
				filesystem::path fullPath = baseDir / (texKey + ext);
				if (filesystem::exists(fullPath))
				{
					TextureMeta meta{};
					meta.fullPath = fullPath.wstring();
					meta.colorSpace = TextureColorSpace::sRGB;
					RegisterTexture(texKey, meta);
					break;
				}
			}
		};

	for (const auto& em : effect.emitters)
	{
		loadTexIfNeeded(em.particle.texKey);     
		loadTexIfNeeded(em.trail.texKey);     
		loadTexIfNeeded(em.trail.spark.texKey);  
	}
}