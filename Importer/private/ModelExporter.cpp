#include "pch.h"
#include "ModelExporter.h"

bool ModelExporter::Export(const ImportedData& data, const filesystem::path& outPath)
{
    outFile.open(outPath, ios::binary | ios::out);
    if (!outFile.is_open())
    {
        assert(false && "failed to read file");
        return false;
    }

    const bool hasSkeletonBlock = data.hasSkeletonBlock;
    WriteData(hasSkeletonBlock);

    WriteMaterials(data.materials);
    WriteMeshes(data.meshes);

    if (hasSkeletonBlock)
    {
        WriteSkeleton(*data.skeleton);
        WriteAnimations(data.animations);
    }

    outFile.close();
    return true;
}

void ModelExporter::WriteMaterials(const vector<unique_ptr<MaterialData>>& materials)
{
    _uint numMaterials = ENUM(materials.size());
    WriteData(numMaterials);

    for (const auto& material : materials)
    {
        WriteString(material->name);

        _uint numTextures = ENUM(material->textures.size());
        WriteData(numTextures);

        for (const auto& pair : material->textures)
        {
            WriteData(pair.first); // TEXTURE_SLOT
            WriteString(pair.second.filename().string()); // 텍스처 경로
        }
    }
}

void ModelExporter::WriteMeshes(const vector<unique_ptr<MeshData>>& meshes)
{
    const _uint numMeshes = ENUM(meshes.size());
    WriteData(numMeshes);

    for (const auto& pMeshData : meshes)
    {
        assert(pMeshData && "WriteMeshes - meshData is null");
        const MeshData& meshData = *pMeshData;

        WriteString(meshData.name);
        WriteData(meshData.materialIdx);

        const _uint layoutTag = ENUM(meshData.layoutID);
        WriteData(layoutTag);

        const _uint typeTag = ENUM(meshData.type);
        WriteData(typeTag);

        switch (meshData.layoutID)
        {
        case VertexLayoutID::PNUTan:
        {
            const _uint vertexStride = ENUM(sizeof(Vertex_PNUTAN));
            const _uint vertexCount = ENUM(meshData.verticesPNUTan.size());
            WriteData(vertexStride);
            WriteData(vertexCount);

            if (vertexCount > 0)
                outFile.write(reinterpret_cast<const char*>(meshData.verticesPNUTan.data()), sizeof(Vertex_PNUTAN) * vertexCount);
            break;
        }

        case VertexLayoutID::PNUTanSkin:
        {
            static_assert(is_trivially_copyable_v<Vertex_Anim>, "Vertex_Anim must be trivally copyable");

            const _uint vertexStride = static_cast<_uint>(sizeof(Vertex_Anim));
            const _uint vertexCount = static_cast<_uint>(meshData.verticesPNUTanSkin.size());
            WriteData(vertexStride);
            WriteData(vertexCount);

            if (vertexCount > 0)
                outFile.write(reinterpret_cast<const char*>(meshData.verticesPNUTanSkin.data()), sizeof(Vertex_Anim) * vertexCount);
            break;
        }

        default:
            assert(false && "WriteMeshes - Unknown VertexLayoutID");
            WriteData(static_cast<_uint>(0));
            WriteData(static_cast<_uint>(0));
            break;
        }
        WriteVector(meshData.indices);
    }
}


void ModelExporter::WriteSkeleton(const SkeletonInfo& skeleton)
{
    _uint numBones = ENUM(skeleton.bones.size());
    WriteData(numBones);

    for (const auto& bone : skeleton.bones)
    {
        WriteString(bone.boneName);
        WriteData(bone.invBindPose);
        WriteData(bone.bindLocal);
        WriteData(bone.parentIdx);
        WriteData(bone.isAnimated);
    }
    WriteData(skeleton.rootBoneIdx);
}

void ModelExporter::WriteAnimations(const vector<unique_ptr<AnimClip>>& animClips)
{
    _uint numClips = ENUM(animClips.size());
    WriteData(numClips);

    for (const auto& clip : animClips)
    {
        WriteString(clip->name);
        WriteData(clip->duration);
        WriteData(clip->tickPerSec);

        // BoneAnim (애니메이션 채널) 개수 쓰기
        _uint numBoneAnims = ENUM(clip->boneAnims.size());
        WriteData(numBoneAnims);

        for (const auto& pair : clip->boneAnims)
        {
            const BoneAnim& boneAnim = pair.second;
            WriteString(boneAnim.boneName);

            // 위치,회전,크기 키프레임 벡터 쓰기
            WriteVector(boneAnim.posKeys);
            WriteVector(boneAnim.rotKeys);
            WriteVector(boneAnim.scaleKeys);
        }
    }
}

void ModelExporter::WriteString(const string& str)
{
    _uint len = (_uint)str.length();
    outFile.write(reinterpret_cast<const char*>(&len), sizeof(_uint));
    outFile.write(str.c_str(), len);
}