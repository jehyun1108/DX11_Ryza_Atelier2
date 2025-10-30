#pragma once

NS_BEGIN(Importer)

class TextureLoader
{
public:
	static unique_ptr<MaterialData> LoadMaterial(const aiMaterial* pMaterial, const aiScene* pScene, const filesystem::path& fbxPath);
	
	static filesystem::path ResolveRelativeToFbx(const filesystem::path& fbxPath, const string& rawPath);

private:
	// 텍스처를 추출하고 .dds 파일로 저장
	static filesystem::path ExtractAndSaveTexture(const aiTexture* texture, const filesystem::path& fbxPath, const string& materialName, const string& textureType);
	static filesystem::path ResolveBestAgainstFbxFolder(const filesystem::path& fbxPath, const string& rawPath);

	static void ProcessTextureType(const aiMaterial* pMaterial, const aiScene* pScene, initializer_list<aiTextureType> assimpTypes, TEXSLOT engineSlot, MaterialData& outMaterial, const filesystem::path& fbxPath, const string& strTextureType);

	// ---------- Debug ------------------------------------
	static const wchar_t* TextureTypeName(aiTextureType type);

	static void DumpAllMaterials(const aiMaterial* pMaterial, const aiScene* pScene, const filesystem::path& fbxPath);
};

NS_END