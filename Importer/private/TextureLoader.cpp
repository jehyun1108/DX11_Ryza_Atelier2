#include "pch.h"
#include "TextureLoader.h"

static bool TryGetFirstTexturePath(const aiMaterial* pMaterial,
    initializer_list<aiTextureType> candidates,
    aiString& outPath,
    aiTextureType& outChosen)
{
    for (aiTextureType type : candidates)
    {
        if (aiReturn_SUCCESS == pMaterial->GetTexture(type, 0, &outPath))
        {
            outChosen = type;
            return true;
        }
    }
    return false;
}

unique_ptr<MaterialData> TextureLoader::LoadMaterial(const aiMaterial* pMaterial, const aiScene* pScene, const filesystem::path& fbxPath)
{
    DumpAllMaterials(pMaterial, pScene, fbxPath);

	auto material = make_unique<MaterialData>();
	material->name = pMaterial->GetName().C_Str();

    // Albedo / BaseColor
    ProcessTextureType(pMaterial, pScene,
        { aiTextureType_BASE_COLOR, aiTextureType_DIFFUSE },
        TEXSLOT::ALBEDO, *material, fbxPath, "Albedo");

    // Normal
    ProcessTextureType(pMaterial, pScene,
        { aiTextureType_NORMALS, aiTextureType_NORMAL_CAMERA, aiTextureType_HEIGHT },
        TEXSLOT::NORMAL, *material, fbxPath, "Normal");

    // Metallic
    ProcessTextureType(pMaterial, pScene,
        { aiTextureType_METALNESS, aiTextureType_NONE },
        TEXSLOT::METALIC, *material, fbxPath, "Metallic");

    // Roughness
    ProcessTextureType(pMaterial, pScene,
        { aiTextureType_DIFFUSE_ROUGHNESS, aiTextureType_SPECULAR },
        TEXSLOT::ROUGHNESS, *material, fbxPath, "Roughness");

    // Emissive
    ProcessTextureType(pMaterial, pScene,
        { aiTextureType_EMISSIVE, aiTextureType_NONE },
        TEXSLOT::EMISSIVE, *material, fbxPath, "Emissive");

    // AO
    ProcessTextureType(pMaterial, pScene,
        { aiTextureType_AMBIENT_OCCLUSION, aiTextureType_NONE },
        TEXSLOT::AO, *material, fbxPath, "AO");

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

filesystem::path TextureLoader::ResolveBestAgainstFbxFolder(const filesystem::path& fbxPath, const string& rawPath)
{
    filesystem::path resolved = TextureLoader::ResolveRelativeToFbx(fbxPath, rawPath);
    if (Utility::FileExists(resolved)) return resolved;

    const filesystem::path baseDir = fbxPath.parent_path();
    const string           fileOnly = filesystem::path(rawPath).filename().string();
    if (!fileOnly.empty())
    {
        filesystem::path sibling = baseDir / fileOnly;
        if (Utility::FileExists(sibling))
            return sibling;
    }

    try
    {
        for (auto it = filesystem::directory_iterator(baseDir); it != filesystem::directory_iterator(); ++it)
        {
            if (it->is_regular_file() && it->path().filename().string() == fileOnly)
                return it->path();
        }
    }
    catch (...) {}

    return {}; 
}

void TextureLoader::ProcessTextureType(const aiMaterial* pMaterial, const aiScene* pScene, initializer_list<aiTextureType> assimpTypes, TEXSLOT engineSlot, MaterialData& outMaterial, const filesystem::path& fbxPath, const string& strTextureType)
{
    aiString foundPath;
    aiTextureType chosenType = aiTextureType_NONE;

    const bool ok = TryGetFirstTexturePath(pMaterial, assimpTypes, foundPath, chosenType);
    if (!ok)
    {
        wstringstream typesW; bool first = true;
        for (aiTextureType t : assimpTypes) { if (!first) typesW << L", "; typesW << TextureTypeName(t); first = false; }
        Utility::Log(L"[Mat:{}] {}: not found (candidates: {})",
            Utility::ToWString(outMaterial.name),
            Utility::ToWString(strTextureType),
            typesW.str().c_str());
        return;
    }

    Utility::Log(L"[Mat:{}] {}: found type={}  path={}",
        Utility::ToWString(outMaterial.name),
        Utility::ToWString(strTextureType),
        TextureTypeName(chosenType),
        Utility::ToWString(foundPath.C_Str()));

    if (foundPath.length == 0) return;

    if (foundPath.C_Str()[0] == '*')
    {
        int textureIndex = 0;
        try { textureIndex = stoi(string(foundPath.C_Str() + 1)); }
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
        // 외부 파일
        filesystem::path resolved = ResolveBestAgainstFbxFolder(fbxPath, foundPath.C_Str());
        outMaterial.textures[engineSlot] = resolved.wstring();

        Utility::Log(L"[Mat:{}] {}: resolved external(type={}) → {}  [{}]",
            Utility::ToWString(outMaterial.name),
            Utility::ToWString(strTextureType),
            TextureTypeName(chosenType),
            resolved.c_str(),
            Utility::FileExists(resolved) ? L"FOUND" : L"NOT FOUND");
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
