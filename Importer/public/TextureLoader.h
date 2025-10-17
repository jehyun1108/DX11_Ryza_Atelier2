#pragma once

NS_BEGIN(Importer)

class TextureLoader
{
public:
	static unique_ptr<MaterialData> LoadMaterial(const aiMaterial* pMaterial, const aiScene* pScene, const filesystem::path& fbxPath);

private:
	// 텍스처를 추출하고 .dds 파일로 저장
	static filesystem::path ExtractAndSaveTexture(const aiTexture* texture, const filesystem::path& fbxPath, const string& materialName, const string& textureType);

	static void ProcessTextureType(const aiMaterial* pMaterial, const aiScene* pScene, aiTextureType assimpType1, aiTextureType assimpType2, TEXSLOT engineSlot, MaterialData& outMaterial, const filesystem::path& fbxPath, const string& strTextureType);

	// ---------- Debug ------------------------------------
	static const wchar_t* TextureTypeName(aiTextureType type);
	static filesystem::path ResolveRelativeToFbx(const filesystem::path& fbxPath, const string& rawPath);
	static void DumpAllMaterials(const aiMaterial* pMaterial, const aiScene* pScene, const filesystem::path& fbxPath);
};

NS_END