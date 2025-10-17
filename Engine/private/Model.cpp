#include "Enginepch.h"

Model::Model()
{
    device  = game.GetDevice();
    context = game.GetContext();
}

shared_ptr<Model> Model::LoadFromFile(const wstring & fullPath)
{
	auto instance = make_shared<Model>();
	if (FAILED(instance->InitFromFile(fullPath)))
		return nullptr;
	return instance;
}

void Model::ResolveMaterials(ShaderCache& shaderCache, TextureCache& textureCache)
{
    for (auto& part : parts)
    {
        if (part.material)
            part.material->Resolve(shaderCache, textureCache);
    }
}

HRESULT Model::InitFromFile(const wstring& fullPath)
{
    ifstream inFile(fullPath, ios::binary);
    if (!inFile.is_open())
    {
        assert(false && "Failed to open model file!");
        return E_FAIL;
    }

    filesystem::path modelDir = filesystem::path(fullPath).parent_path();

    Utility::ReadData(inFile, isSkeletalModel);
    ReadMaterials(inFile, modelDir);
    ReadMeshes(inFile);

    if (isSkeletalModel)
    {
        ReadSkeletons(inFile);
        ReadAnimations(inFile);
    }

    FinalSetUp();
    inFile.close();
    return S_OK;
}

void Model::ReadMaterials(ifstream& inFile, const filesystem::path& modelDir)
{
    auto& assets = game.GetAssetSystem();

    const wstring baseKey = logicalKey.empty() ? Utility::Normalize(modelDir.filename().wstring()) : logicalKey;

    _uint numMaterials{}; Utility::ReadData(inFile, numMaterials);
    materials.resize(numMaterials);

    for (auto& material : materials)
    {
        material = make_shared<Material>();

        string materialName; Utility::ReadString(inFile, materialName);
        _uint numTextures;   Utility::ReadData(inFile, numTextures);

        for (_uint j = 0; j < numTextures; ++j)
        {
            TEXSLOT slot;       Utility::ReadData(inFile, slot);
            string texFileName; Utility::ReadString(inFile, texFileName);
            
            // Default 처리
            if (texFileName.empty() || texFileName == "default")
            {
                auto [defaultKey, _] = Utility::GetDefaultTex(slot);
                material->SetTextureKey(slot, defaultKey, SHADER::PS);
                continue;
            }

            const wstring wTexFileName = Utility::ToWString(texFileName);
            const wstring texFullPath = (modelDir / wTexFileName).wstring();

            // 파일 없으면 Default
            if (!filesystem::exists(texFullPath))
            {
                auto [defaultKey, _] = Utility::GetDefaultTex(slot);
                material->SetTextureKey(slot, defaultKey, SHADER::PS);
                continue;
            }
            
            const wstring texStem = Utility::Normalize(filesystem::path(wTexFileName).stem().wstring());
            const wstring texKey = baseKey + L"/" + texStem;

            const TextureColorSpace colorSpace = Utility::SlotToColorSpace(slot);

            assets.RegisterTexture(texKey, { texFullPath, colorSpace });
            material->SetTextureKey(slot, texKey, SHADER::PS);
        }
    }
}

void Model::ReadMeshes(ifstream& inFile)
{
    _uint numMeshes; Utility::ReadData(inFile, numMeshes);
    parts.reserve(numMeshes);

    for (_uint i = 0; i < numMeshes; ++i)
    {
        string meshName;      Utility::ReadString(inFile, meshName);
        _uint  materialIdx;   Utility::ReadData(inFile, materialIdx);

        _uint  layoutTag = 0; Utility::ReadData(inFile, layoutTag);
        const VertexLayoutID layoutID = static_cast<VertexLayoutID>(layoutTag);

        _uint typeTag = 0;   Utility::ReadData(inFile, typeTag);
        const MESHTYPE usage = static_cast<MESHTYPE>(typeTag);

        _uint vertexStride = 0; Utility::ReadData(inFile, vertexStride);
        _uint vertexCount  = 0; Utility::ReadData(inFile, vertexCount);

        vector<std::byte> vertexBytes;
        vertexBytes.resize(static_cast<size_t>(vertexStride) * vertexCount);
        if (vertexCount > 0)
            inFile.read(reinterpret_cast<char*>(vertexBytes.data()), static_cast<streamsize>(vertexBytes.size()));

        vector<_uint> indices32; Utility::ReadVector(inFile, indices32);

        // MeshMeta  ----------------------------

        MeshMeta meta{};
        meta.key      = Utility::ToWString(meshName);
        meta.source   = L"";
        meta.layout   = layoutID;
        meta.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        meta.usage    = usage;
        meta.meshKind = (layoutID == VertexLayoutID::PNUTanSkin) ? MESH::Skeletal : MESH::Static;
        // IdxFmt
        vector<uint16_t> indices16;
       
        _uint maxIdx = 0;
        for (auto v : indices32)
            maxIdx = max(maxIdx, v);
        const bool canUse16 = (maxIdx <= 65535u);

        if (canUse16)
        {
            indices16.resize(indices32.size());
            for (size_t j = 0; j < indices32.size(); ++j)
                indices16[j] = static_cast<uint16_t>(indices32[j]);
            meta.idxFmt = DXGI_FORMAT_R16_UINT;
        }
        else
            meta.idxFmt = DXGI_FORMAT_R32_UINT;

        // Bounding
        // meta.

        // Mesh ------------------------------------------------
        shared_ptr<Mesh> mesh = make_shared<Mesh>();
        if (canUse16)
            HR(mesh->InitFromBuffers(device, meta, vertexBytes.data(), vertexCount, vertexStride, indices16.data(), (_uint)indices16.size()));
        if (!canUse16)
            HR(mesh->InitFromBuffers(device, meta, vertexBytes.data(), vertexCount, vertexStride, indices32.data(), (_uint)indices32.size()));

        // Parts
        ModelParts part;
        part.mesh = mesh;
        if (materialIdx < materials.size())
            part.material = materials[materialIdx];

        parts.emplace_back(move(part));
    }
}

void Model::ReadSkeletons(ifstream& inFile)
{
    auto skeletonInfo = make_shared<SkeletonInfo>();

    _uint numBones; Utility::ReadData(inFile, numBones);

    skeletonInfo->bones.resize(numBones);

    for (_uint i = 0; i < numBones; ++i)
    {
        BoneInfo& boneInfo = skeletonInfo->bones[i];

        Utility::ReadString(inFile, boneInfo.boneName);
        Utility::ReadData(inFile, boneInfo.invBindPose);
        Utility::ReadData(inFile, boneInfo.bindLocal);
        Utility::ReadData(inFile, boneInfo.parentIdx);
        Utility::ReadData(inFile, boneInfo.isAnimated);

        skeletonInfo->boneNameToIdx[boneInfo.boneName] = i;
    }
    Utility::ReadData(inFile, skeletonInfo->rootBoneIdx);
    skeleton = make_shared<Skeleton>(*skeletonInfo);
}

void Model::ReadAnimations(ifstream& inFile)
{
    _uint numClips; Utility::ReadData(inFile, numClips);
    animClips.reserve(numClips);

    for (_uint i = 0; i < numClips; ++i)
    {
        auto clip = make_shared<AnimClip>();
        Utility::ReadString(inFile, clip->name);
        Utility::ReadData(inFile, clip->duration);
        Utility::ReadData(inFile, clip->tickPerSec);

        _uint numBoneAnims; Utility::ReadData(inFile, numBoneAnims);
        for (_uint j = 0; j < numBoneAnims; ++j)
        {
            BoneAnim boneAnim;
            Utility::ReadString(inFile, boneAnim.boneName);

            Utility::ReadVector(inFile, boneAnim.posKeys);
            Utility::ReadVector(inFile, boneAnim.rotKeys);
            Utility::ReadVector(inFile, boneAnim.scaleKeys);
            clip->boneAnims[boneAnim.boneName] = move(boneAnim);
        }
        animClips.push_back(clip);
    }
}

void Model::FinalSetUp()
{
    // Shader 정리
    for (auto& part : parts)
    {
        const Mesh* mesh = part.mesh.get();
        if (!mesh) continue;

        const VertexLayoutID layout = mesh->GetLayoutID();
        const wchar_t* shaderKey = (layout == VertexLayoutID::PNUTanSkin) ? L"PNUTanSkin" : L"PNUTan";

        auto mtrl = part.material ? part.material->Clone() : make_shared<Material>();
        mtrl->SetShaderKey(shaderKey);
        part.material = mtrl;
    }
    vector<_float3> fallbackPos{};
    fallbackPos.reserve(4096);

    // Bounding
    bool hasBoundingBox = false;
    BoundingBox merged{};
    auto Merge = [&](const BoundingBox& box)
        {
            if (!hasBoundingBox)
            {
                merged = box;
                hasBoundingBox = true;
            }
            else
                BoundingBox::CreateMerged(merged, merged, box);
        };

    for (const auto& part : parts)
    {
        if (part.mesh && part.mesh->HasLocalBounds())
            Merge(part.mesh->GetLocalAABB());
    }

    if (hasBoundingBox)
        boundingBox = merged;

    // AnimClip
    clipTable.clear();
    if (isSkeletalModel)
    {
        for (auto& clip : animClips)
        {
            wstring name = Utility::ToWString(clip->name);
            clipTable[name] = clip.get();
        }
    }
}