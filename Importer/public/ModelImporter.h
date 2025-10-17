#pragma once


NS_BEGIN(Importer)

class ModelImporter
{
public:
	unique_ptr<ImportedData> Import(const filesystem::path& fbxPath);

private:
	static _float4x4 ConvertAssimpMatrix(const aiMatrix4x4 aiMat);

	void ParseAnimations(ImportedData& outData);
	void ParseSkeletons(ImportedData& outData);
	void ParseMeshes(ImportedData& outData);

	void ParseMaterials(ImportedData& outData, const filesystem::path& fbxPath);
	void ParseSkeleton_Recursive(aiNode* node, int parentIdx, SkeletonInfo& outSkeleton, unordered_set<string>& realBoneNames);

	void ParseVerticesPNUTanSkin(const aiMesh* mesh, MeshData& outMesh);
	void ParseVerticesPNUTan(const aiMesh* mesh, MeshData& outMesh);

	void ParseWeights(const aiMesh* mesh, const SkeletonInfo& skeleton, vector<vector<pair<_uint, float>>>& outTempWeights);
	void ParseIndices(const aiMesh* mesh, MeshData& outMesh);

private:
	template<typename T>
	static void AppendVertex(vector<std::byte>& blob, const T& vertex)
	{
		const auto* p = reinterpret_cast<const std::byte*>(&vertex);
		blob.insert(blob.end(), p, p + sizeof(T));
	}

	float ComputeTangentSign(const aiVector3D& normalAi, const aiVector3D& tangentAi, const aiVector3D& bitangentAi);

private:
	const aiScene* scene{};
};

NS_END