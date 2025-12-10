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


const Mesh* Model::GetFirstMesh() const
{
    for (const auto& p : parts)
        if (p.mesh) return p.mesh.get();
    return nullptr;
}

const Mesh* Model::GetLargestMeshByAABB() const
{
    const Mesh* best = nullptr;
    float bestVol = -1.f;

    for (const auto& p : parts)
    {
        if (!p.mesh || !p.mesh->HasLocalBounds()) continue;
        const auto& a = p.mesh->GetLocalAABB();
        const float vol = (a.Extents.x * 2.f) * (a.Extents.y * 2.f) * (a.Extents.z * 2.f);
        if (vol > bestVol) { bestVol = vol; best = p.mesh.get(); }
    }
    if (best) return best;

    return GetFirstMesh();
}

void Model::GetAllMeshes(vector<const Mesh*>& out) const
{
    out.clear();
    out.reserve(parts.size());
    for (const auto& p : parts)
        if (p.mesh) out.push_back(p.mesh.get());
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

    Utility::ReadData(inFile, isSkinned);
    ReadMaterials(inFile, modelDir);
    ReadMeshes(inFile);

    if (isSkinned)
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
    auto& assets = game.GetRegistry().Get<AssetSystem>();
    const wstring baseKey = logicalKey.empty()? Utility::Normalize(modelDir.filename().wstring()) : logicalKey;

    _uint numMaterials{};
    Utility::ReadData(inFile, numMaterials);
    materials.resize(numMaterials);

    for (auto& material : materials)
    {
        material = make_shared<Material>();

        string materialName;
        Utility::ReadString(inFile, materialName);
        _uint numTextures;
        Utility::ReadData(inFile, numTextures);

        wstring diffuseStem;

        for (_uint j = 0; j < numTextures; ++j)
        {
            TEXSLOT slot;
            Utility::ReadData(inFile, slot);
            string texFileName;
            Utility::ReadString(inFile, texFileName);

            // Default 처리
            if (texFileName.empty() || texFileName == "default")
            {
                auto [defaultKey, _] = Utility::GetDefaultTex(slot);
                material->SetTextureKey(slot, defaultKey, SHADER::PS);
                continue;
            }

            const wstring wTexFileName = Utility::ToWString(texFileName);
            const filesystem::path texPath = modelDir / wTexFileName;
            const wstring texFullPath = texPath.wstring();

            // 파일 없으면 Default
            if (!filesystem::exists(texFullPath))
            {
                auto [defaultKey, _] = Utility::GetDefaultTex(slot);
                material->SetTextureKey(slot, defaultKey, SHADER::PS);
                continue;
            }

            const wstring texStem = Utility::Normalize(texPath.stem().wstring());
            const wstring texKey = baseKey + L"/" + texStem;

            const TextureColorSpace colorSpace = Utility::SlotToColorSpace(slot);
            assets.RegisterTexture(texKey, { texFullPath, colorSpace });
            material->SetTextureKey(slot, texKey, SHADER::PS);

            if (slot == TEXSLOT::ALBEDO)
                diffuseStem = texStem;
        }

        if (isSkinned)
        {
            ApplyDefaultSkinMapping(baseKey, diffuseStem, *material, modelDir);

            if (IsSpecialCharacter(baseKey))
                ApplySpecialMapping(baseKey, diffuseStem, *material, modelDir);
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

void Model::TryNormalFromDiffuseMap(Material& targetMaterial, const wstring& diffuseFullPath, const wstring& baseKey)
{
    if (diffuseFullPath.empty()) return;

    const filesystem::path sibling = Utility::MakeNormalMapPath(diffuseFullPath);
    if (sibling.empty()) return;
    if (Utility::FileExists(sibling)) return;

    auto& assets = game.GetRegistry().Get<AssetSystem>();

    const wstring normalStem = Utility::Normalize(sibling.stem().wstring());
    const wstring normalKey = baseKey + L"/" + normalStem;

    assets.RegisterTexture(normalKey, { sibling.wstring(), TextureColorSpace::Linear });
    targetMaterial.SetTextureKey(TEXSLOT::NORMAL, normalKey, SHADER::PS);
}

void Model::BuildBindPose(const Skeleton& skeleton, vector<_float4x4>& out)
{
    const size_t boneCount = skeleton.bonesByIdx.size();
    out.resize(boneCount);

    vector<_float4x4> combinedBind(boneCount);
    for (size_t i = 0; i < boneCount; ++i)
        XMStoreFloat4x4(&combinedBind[i], XMMatrixIdentity());

    function<void(Bone*, const _float4x4&)> dfs;
    dfs = [&](Bone* curBone, const _float4x4& parentCombined)
        {
            _mat parentMat   = XMLoadFloat4x4(&parentCombined);
            _mat localMat    = XMLoadFloat4x4(&curBone->bindLocal);
            _mat combinedMat = XMMatrixMultiply(parentMat, localMat);
            XMStoreFloat4x4(&combinedBind[curBone->idx], combinedMat);

            for (Bone* child : curBone->children)
                dfs(child, combinedBind[curBone->idx]);
        };

    _float4x4 identityMat{};
    XMStoreFloat4x4(&identityMat, XMMatrixIdentity());

    if (skeleton.rootBone)
        dfs(skeleton.rootBone, identityMat);

    for (size_t i = 0;  i < boneCount; ++i)
    {
        const _float4x4& combinedBindMat = combinedBind[i];
        const _float4x4& invBindPose     = skeleton.bonesByIdx[i]->invBindPose;

        const _mat combinedMat = XMLoadFloat4x4(&combinedBindMat);
        const _mat invBindMat  = XMLoadFloat4x4(&invBindPose);
        const _mat skinMat     = XMMatrixMultiply(combinedMat, invBindMat);

        XMStoreFloat4x4(&out[i], skinMat);
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
    if (isSkinned)
    {
        for (auto& clip : animClips)
        {
            wstring name = Utility::ToWString(clip->name);
            clipTable[name] = clip.get();
        }
    }
    if (skeleton)
        BuildBindPose(*skeleton, bindPoseMatrices);
}

void Model::ApplyDefaultSkinMapping(const wstring& baseKey, const wstring& diffuseStem, Material& material, const filesystem::path& modelDir)
{
    auto& assets = game.GetRegistry().Get<AssetSystem>();

    if (diffuseStem == L"0")
    {
        filesystem::path normalPath = modelDir / L"1.dds";
        assert(filesystem::exists(normalPath) && L"1.dds normal map not found!");

        wstring nStem = Utility::Normalize(normalPath.stem().wstring());
        wstring normalKey = baseKey + L"/" + nStem;
        TextureColorSpace nCs = Utility::SlotToColorSpace(TEXSLOT::NORMAL);
        assets.RegisterTexture(normalKey, { normalPath.wstring(), nCs });
        material.SetTextureKey(TEXSLOT::NORMAL, normalKey, SHADER::PS);

        filesystem::path aoPath = modelDir / L"2.dds";
        assert(filesystem::exists(aoPath) && L"2.dds AO map not found!");

        wstring aStem = Utility::Normalize(aoPath.stem().wstring());
        wstring aoKey = baseKey + L"/" + aStem;
        TextureColorSpace aCs = Utility::SlotToColorSpace(TEXSLOT::AO);
        assets.RegisterTexture(aoKey, { aoPath.wstring(), aCs });
        material.SetTextureKey(TEXSLOT::AO, aoKey, SHADER::PS);

        filesystem::path maskPath = modelDir / L"3.dds";
        assert(filesystem::exists(maskPath) && L"3.dds material mask not found!");

        wstring mStem = Utility::Normalize(maskPath.stem().wstring());
        wstring maskKey = baseKey + L"/" + mStem;

        TextureColorSpace mCs = Utility::SlotToColorSpace(TEXSLOT::MATMASK);
        assets.RegisterTexture(maskKey, { maskPath.wstring(), mCs });
        material.SetTextureKey(TEXSLOT::MATMASK, maskKey, SHADER::PS);
    }
    else
    {
        auto [normalKey, _0] = Utility::GetDefaultTex(TEXSLOT::NORMAL);
        auto [aoKey, _1] = Utility::GetDefaultTex(TEXSLOT::AO);
        auto [maskKey, _2] = Utility::GetDefaultTex(TEXSLOT::MATMASK);

        material.SetTextureKey(TEXSLOT::NORMAL, normalKey, SHADER::PS);
        material.SetTextureKey(TEXSLOT::AO, aoKey, SHADER::PS);
        material.SetTextureKey(TEXSLOT::MATMASK, maskKey, SHADER::PS);
    }
}

void Model::ApplySpecialMapping(const wstring& baseKey, const wstring& diffuseStem, Material& material, const filesystem::path& dir)
{
    auto& assets = game.GetRegistry().Get<AssetSystem>();

    auto setTex = [&](TEXSLOT slot, const wchar_t* stemLiteral)
        {
            wstring stemStr = stemLiteral;                  // L"5" 같은 것
            filesystem::path texPath = dir / (stemStr + L".dds");
            assert(filesystem::exists(texPath) && L"special mapping texture not found!");

            wstring normStem = Utility::Normalize(texPath.stem().wstring());
            wstring texKey = baseKey + L"/" + normStem;

            TextureColorSpace cs = Utility::SlotToColorSpace(slot);
            assets.RegisterTexture(texKey, { texPath.wstring(), cs });
            material.SetTextureKey(slot, texKey, SHADER::PS);
        };

    if (baseKey == L"ryza")
    {
        if (diffuseStem == L"10")
        {
            setTex(TEXSLOT::AO, L"11");
        }
        else if (diffuseStem == L"13")
        {
            setTex(TEXSLOT::AO, L"15");
        }
        else if (diffuseStem == L"17")
        {
            setTex(TEXSLOT::AO, L"20");
        }
    }
    else if (baseKey == L"patricia")
    {
        if (diffuseStem == L"4")
        {
            setTex(TEXSLOT::AO, L"5");
        }
        else if (diffuseStem == L"7")
        {
            setTex(TEXSLOT::AO, L"9");
        }
        else if (diffuseStem == L"11")
        {
            setTex(TEXSLOT::AO, L"14");
            setTex(TEXSLOT::NORMAL, L"15");
        }
    }
    else if (baseKey == L"klaudia")
    {
        if (diffuseStem == L"4")
        {
            setTex(TEXSLOT::AO, L"5");
        }
        else if (diffuseStem == L"7")
        {
            setTex(TEXSLOT::AO, L"9");
        }
        else if (diffuseStem == L"11")
        {
            setTex(TEXSLOT::AO, L"14");
        }
    }
}