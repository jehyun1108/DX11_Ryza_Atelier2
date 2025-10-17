#include "pch.h"
#include "TextureLoader.h"

unique_ptr<MaterialData> TextureLoader::LoadMaterial(const aiMaterial* pMaterial, const aiScene* pScene, const filesystem::path& fbxPath)
{
    DumpAllMaterials(pMaterial, pScene, fbxPath);

	auto material = make_unique<MaterialData>();
	material->name = pMaterial->GetName().C_Str();

	ProcessTextureType(pMaterial, pScene, aiTextureType_BASE_COLOR, aiTextureType_DIFFUSE, TEXSLOT::ALBEDO, *material, fbxPath, "Albedo");
	ProcessTextureType(pMaterial, pScene, aiTextureType_NORMALS, aiTextureType_NONE, TEXSLOT::NORMAL, *material, fbxPath, "Normal");
	ProcessTextureType(pMaterial, pScene, aiTextureType_METALNESS, aiTextureType_NONE, TEXSLOT::METALIC, *material, fbxPath, "Metallic");
	ProcessTextureType(pMaterial, pScene, aiTextureType_DIFFUSE_ROUGHNESS, aiTextureType_SPECULAR, TEXSLOT::ROUGHNESS, *material, fbxPath, "Roughness");
	ProcessTextureType(pMaterial, pScene, aiTextureType_EMISSIVE, aiTextureType_NONE, TEXSLOT::EMISSIVE, *material, fbxPath, "Emissive");
	ProcessTextureType(pMaterial, pScene, aiTextureType_AMBIENT_OCCLUSION, aiTextureType_NONE, TEXSLOT::AO, *material, fbxPath, "AO");

	return material;
}

filesystem::path TextureLoader::ExtractAndSaveTexture(const aiTexture* texture, const filesystem::path& fbxPath, const string& materialName, const string& textureType)
{
    ScratchImage image;
    HRESULT hr;

    if (texture->mHeight == 0)
        hr = LoadFromWICMemory(reinterpret_cast<const uint8_t*>(texture->pcData), texture->mWidth, WIC_FLAGS_NONE, nullptr, image);
    else
    {
        Image rawImage;
        rawImage.width = texture->mWidth;
        rawImage.height = texture->mHeight;
        rawImage.format = DXGI_FORMAT_B8G8R8A8_UNORM;
        rawImage.rowPitch = texture->mWidth * 4;
        rawImage.slicePitch = rawImage.rowPitch * texture->mHeight;
        rawImage.pixels = reinterpret_cast<uint8_t*>(texture->pcData);
        hr = image.InitializeFromImage(rawImage);
    }
    if (FAILED(hr)) return {};

    if (textureType == "Albedo" || textureType == "Emissive")
        image.OverrideFormat(MakeSRGB(image.GetMetadata().format));

    ScratchImage mipChain;
    hr = GenerateMipMaps(*image.GetImage(0, 0, 0), TEX_FILTER_DEFAULT, 0, mipChain);

    if (FAILED(hr))
        mipChain = move(image);

    filesystem::path saveDir = fbxPath.parent_path();
    string newFileName = fbxPath.stem().string() + "_" + materialName + "_" + textureType + ".dds";
    filesystem::path savePath = saveDir / newFileName;

    hr = SaveToDDSFile(*mipChain.GetImage(0, 0, 0), DDS_FLAGS_NONE, savePath.c_str());
    if (FAILED(hr)) return {};

    return savePath;
}

void TextureLoader::ProcessTextureType(const aiMaterial* pMaterial, const aiScene* pScene, aiTextureType assimpType1, aiTextureType assimpType2, TEXSLOT engineSlot, MaterialData& outMaterial, const filesystem::path& fbxPath, const string& strTextureType)
{
    aiString path;
    const bool foundFirst = (aiReturn_SUCCESS == pMaterial->GetTexture(assimpType1, 0, &path));
    const bool foundSecond = (!foundFirst && assimpType2 != aiTextureType_NONE &&
        aiReturn_SUCCESS == pMaterial->GetTexture(assimpType2, 0, &path));

    if (!foundFirst && !foundSecond)
    {
        Utility::Log(L"[Mat:{}] {}: not found (types: {}, {})",
            Utility::ToWString(outMaterial.name),
            Utility::ToWString(strTextureType),
            TextureTypeName(assimpType1),
            TextureTypeName(assimpType2));
        return;
    }

    const aiTextureType chosenType = foundFirst ? assimpType1 : assimpType2;
    Utility::Log(L"[Mat:{}] {}: found type={}  path={}",
        Utility::ToWString(outMaterial.name),
        Utility::ToWString(strTextureType),
        TextureTypeName(chosenType),
        Utility::ToWString(path.C_Str()));

    if (path.length == 0) return;

    if (path.C_Str()[0] == '*')
    {
        int textureIndex = 0;
        try { textureIndex = stoi(string(path.C_Str() + 1)); }
        catch (...) {}

        if (!pScene || textureIndex < 0 || textureIndex >= (int)pScene->mNumTextures)
        {
            Utility::Log(L"[Mat:{}] {}: embedded index {} out of range",
                Utility::ToWString(outMaterial.name),
                Utility::ToWString(strTextureType),
                textureIndex);
            return;
        }

        const aiTexture* texture = pScene->mTextures[textureIndex];
        filesystem::path newPath = ExtractAndSaveTexture(texture, fbxPath, outMaterial.name, strTextureType);
        if (!newPath.empty())
        {
            outMaterial.textures[engineSlot] = newPath.wstring();
            Utility::Log(L"[Mat:{}] {}: embedded extracted → {}",
                Utility::ToWString(outMaterial.name),
                Utility::ToWString(strTextureType),
                newPath.c_str());
        }
    }
    else
    {
        // 외부 경로는 FBX 기준 상대경로로 정규화해 저장
        filesystem::path resolved = ResolveRelativeToFbx(fbxPath, path.C_Str());
        outMaterial.textures[engineSlot] = resolved.wstring();
        Utility::Log(L"[Mat:{}] {}: resolved external → {}  [{}]",
            Utility::ToWString(outMaterial.name),
            Utility::ToWString(strTextureType),
            resolved.c_str(),
            filesystem::exists(resolved) ? L"FOUND" : L"NOT FOUND");
    }
}

const wchar_t* TextureLoader::TextureTypeName(aiTextureType type)
{
    switch (type)
    {
    case aiTextureType_NONE:             return L"NONE";
    case aiTextureType_DIFFUSE:          return L"DIFFUSE";
    case aiTextureType_SPECULAR:         return L"SPECULAR";
    case aiTextureType_AMBIENT:          return L"AMBIENT";
    case aiTextureType_EMISSIVE:         return L"EMISSIVE";
    case aiTextureType_HEIGHT:           return L"HEIGHT";
    case aiTextureType_NORMALS:          return L"NORMALS";
    case aiTextureType_SHININESS:        return L"SHININESS";
    case aiTextureType_OPACITY:          return L"OPACITY";
    case aiTextureType_DISPLACEMENT:     return L"DISPLACEMENT";
    case aiTextureType_LIGHTMAP:         return L"LIGHTMAP";
    case aiTextureType_REFLECTION:       return L"REFLECTION";
    case aiTextureType_BASE_COLOR:       return L"BASE_COLOR";
    case aiTextureType_NORMAL_CAMERA:    return L"NORMAL_CAMERA";
    case aiTextureType_EMISSION_COLOR:   return L"EMISSION_COLOR";
    case aiTextureType_METALNESS:        return L"METALNESS";
    case aiTextureType_DIFFUSE_ROUGHNESS:return L"DIFFUSE_ROUGHNESS";
    case aiTextureType_AMBIENT_OCCLUSION:return L"AMBIENT_OCCLUSION";
    default:                             return L"(UNKNOWN)";
    }
}

filesystem::path TextureLoader::ResolveRelativeToFbx(const filesystem::path& fbxPath, const string& rawPath)
{
    string cleaned = rawPath;
    if (cleaned.rfind("file://", 0) == 0) cleaned = cleaned.substr(7);
    for (char& ch : cleaned) if (ch == '\\') ch = '/';

    filesystem::path baseDir = fbxPath.parent_path();
    filesystem::path fullPath = baseDir / cleaned;

    error_code ec;
    filesystem::path normalized = filesystem::weakly_canonical(fullPath, ec);
    return ec ? fullPath : normalized;
}

void TextureLoader::DumpAllMaterials(const aiMaterial* pMaterial, const aiScene* pScene, const filesystem::path& fbxPath)
{
    const string matName = pMaterial->GetName().C_Str();
    Utility::Log(L"[Assimp] ==== Materials : {} ====", Utility::ToWString(matName));

    const aiTextureType typesToCheck[] =
    {
        aiTextureType_BASE_COLOR, aiTextureType_DIFFUSE,
        aiTextureType_NORMALS, aiTextureType_NORMAL_CAMERA, aiTextureType_HEIGHT,
        aiTextureType_METALNESS, aiTextureType_DIFFUSE_ROUGHNESS, aiTextureType_SPECULAR,
        aiTextureType_AMBIENT_OCCLUSION, aiTextureType_EMISSIVE,
        aiTextureType_DISPLACEMENT, aiTextureType_OPACITY
    };

    for (aiTextureType type : typesToCheck)
    {
        const unsigned count = pMaterial->GetTextureCount(type);
        if (count == 0) continue;

        Utility::Log(L"  [{}] count={}", TextureTypeName(type), count);

        for (unsigned i = 0; i < count; ++i)
        {
            aiString aiPath;
            if (aiReturn_SUCCESS != pMaterial->GetTexture(type, i, &aiPath))
            {
                Utility::Log(L"    #{} : <GetTexture FAILED>", i);
                continue;
            }

            const char* cpath = aiPath.C_Str();
            if (!cpath || *cpath == '\0')
            {
                Utility::Log(L"    #{} : <EMPTY PATH>", i);
                continue;
            }

            if (cpath[0] == '*')
            {
                int embeddedIndex = 0;
                try { embeddedIndex = stoi(string(cpath + 1)); }
                catch (...) {}
                Utility::Log(L"    #{} : embedded *{} (type={})", i, embeddedIndex, TextureTypeName(type));

                if (pScene && embeddedIndex >= 0 && embeddedIndex < (int)pScene->mNumTextures)
                {
                    const aiTexture* tex = pScene->mTextures[embeddedIndex];
                    Utility::Log(L"           embedded size: mWidth={} mHeight={} (compressed={})",
                        tex->mWidth, tex->mHeight, (tex->mHeight == 0 ? L"YES" : L"NO"));
                }
            }
            else
            {
                filesystem::path resolved = ResolveRelativeToFbx(fbxPath, cpath);
                const bool existsOnDisk = filesystem::exists(resolved);
                Utility::Log(L"    #{} : {}  (resolved: {})  [{}]",
                    i, Utility::ToWString(cpath), resolved.c_str(),
                    existsOnDisk ? L"FOUND" : L"NOT FOUND");
            }
        }
    }
}
