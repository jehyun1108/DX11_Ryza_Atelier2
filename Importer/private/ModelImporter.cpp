#include "pch.h"
#include "ModelImporter.h"

#include "TextureLoader.h"

_float4x4 ModelImporter::ConvertAssimpMatrix(const aiMatrix4x4 aiMat)
{
	_float4x4 result;
	memcpy(&result, &aiMat, sizeof(_float4x4));
	_mat dxMat = XMLoadFloat4x4(&result);
	dxMat = XMMatrixTranspose(dxMat);
	XMStoreFloat4x4(&result, dxMat);
	return result;
}

unique_ptr<ImportedData> ModelImporter::Import(const filesystem::path& fbxPath)
{
	 unique_ptr<Assimp::Importer> importer = make_unique<Assimp::Importer>();
	_uint flag = aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast;

	scene = importer->ReadFile(fbxPath.string(), flag);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		assert(false);
		return nullptr;
	}

	auto outData = make_unique<ImportedData>();
	ParseMaterials(*outData, fbxPath);

	bool anyMeshSkinned = false;
	for (_uint i = 0; i < scene->mNumMeshes; ++i)
	{
		if (scene->mMeshes[i]->mNumBones > 0)
		{
			anyMeshSkinned = true;
			break;
		}
	}
	outData->hasSkeletonBlock = anyMeshSkinned;

	if (outData->hasSkeletonBlock)
		ParseSkeletons(*outData);

	ParseMeshes(*outData);

	if (outData->hasSkeletonBlock)
		ParseAnimations(*outData);

	// ------------------------------------------------------------
	return outData;
}

void ModelImporter::ParseSkeletons(ImportedData& outData)
{
	outData.skeleton = make_unique<SkeletonInfo>();
	SkeletonInfo& outSkeleton = *outData.skeleton;

	unordered_set<string> realBoneNames;
	for (_uint i = 0; i < scene->mNumMeshes; ++i)
	{
		const aiMesh* mesh = scene->mMeshes[i];
		for (_uint j = 0; j < mesh->mNumBones; ++j)
			realBoneNames.insert({ mesh->mBones[j]->mName.C_Str() });
	}

	ParseSkeleton_Recursive(scene->mRootNode, -1, outSkeleton, realBoneNames);

	for (_uint i = 0; i < scene->mNumMeshes; ++i)
	{
		const aiMesh* mesh = scene->mMeshes[i];
		for (_uint j = 0; j < mesh->mNumBones; ++j)
		{
			const aiBone* bone = mesh->mBones[j];
			const string boneName = bone->mName.C_Str();

			auto it = outSkeleton.boneNameToIdx.find(boneName);
			if (it != outSkeleton.boneNameToIdx.end())
				outSkeleton.bones[it->second].invBindPose = ConvertAssimpMatrix(bone->mOffsetMatrix);
		}
	}

	auto rootIter = find_if(outSkeleton.bones.begin(), outSkeleton.bones.end(), [](const BoneInfo& bone) {return bone.parentIdx == -1; });
	outSkeleton.rootBoneIdx = (rootIter != outSkeleton.bones.end()) ? outSkeleton.boneNameToIdx[rootIter->boneName] : -1;
	assert(outSkeleton.rootBoneIdx >= 0 && "Root bone not found");
}

void ModelImporter::ParseSkeleton_Recursive(aiNode* node, int parentIdx, SkeletonInfo& outSkeleton, unordered_set<string>& boneNames)
{
	string nodeName = node->mName.C_Str();
	int nextParentIdx = parentIdx;

	if (boneNames.count(nodeName))
	{
		BoneInfo boneInfo;
		boneInfo.boneName = node->mName.C_Str();
		boneInfo.parentIdx = parentIdx;
		boneInfo.bindLocal = ConvertAssimpMatrix(node->mTransformation);
		XMStoreFloat4x4(&boneInfo.invBindPose, XMMatrixIdentity());

		_uint boneIdx = ENUM(outSkeleton.bones.size());
		outSkeleton.boneNameToIdx[boneInfo.boneName] = boneIdx;
		outSkeleton.bones.push_back(boneInfo);
		 
		nextParentIdx = (int)boneIdx;
	}
		for (_uint i = 0; i < node->mNumChildren; ++i)
			ParseSkeleton_Recursive(node->mChildren[i], nextParentIdx, outSkeleton, boneNames);
}

void ModelImporter::ParseVerticesPNUTanSkin(const aiMesh* mesh, MeshData& outMesh)
{
	outMesh.verticesPNUTanSkin.resize(mesh->mNumVertices);

	const bool hasPos     = mesh->HasPositions();
	const bool hasNormal  = mesh->HasNormals();
	const bool hasTangent = mesh->HasTangentsAndBitangents();
	const bool hasUV0     = mesh->HasTextureCoords(0);

	for (_uint i = 0; i < mesh->mNumVertices; ++i)
	{
		auto& vertex = outMesh.verticesPNUTanSkin[i];

		if (hasPos)    vertex.pos = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
		if (hasNormal) vertex.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
		if (hasUV0)    vertex.uv = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };

		// Tangent.w = sign (handedness)
		if (hasTangent)
		{
			const aiVector3D& T = mesh->mTangents[i];
			const aiVector3D& B = mesh->mBitangents[i];
			const aiVector3D& N = mesh->mNormals[i];

			const float sign = ComputeTangentSign(N, T, B);
			vertex.tangent = { T.x, T.y, T.z, sign };
		}
		else
			vertex.tangent = { 1.f, 0.f, 0.f, 1.f };

		for (int k = 0; k < 4; ++k)
		{
			vertex.boneIndices[k] = 0;
			vertex.boneWeights[k] = 0.f;
		}
	}
}

void ModelImporter::ParseVerticesPNUTan(const aiMesh* mesh, MeshData& outMesh)
{
	outMesh.verticesPNUTan.resize(mesh->mNumVertices);

	const bool hasPos     = mesh->HasPositions();
	const bool hasNormal  = mesh->HasNormals();
	const bool hasTangent = mesh->HasTangentsAndBitangents();
	const bool hasUV0     = mesh->HasTextureCoords(0);

	for (_uint i = 0; i < mesh->mNumVertices; ++i)
	{
		auto& vertex = outMesh.verticesPNUTan[i];

		if (hasPos)    vertex.pos    = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
		if (hasNormal) vertex.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
		if (hasUV0)    vertex.uv     = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };

		// Tangent.w = sign (handedness)
		if (hasTangent)
		{
			const aiVector3D& T = mesh->mTangents[i];
			const aiVector3D& B = mesh->mBitangents[i];
			const aiVector3D& N = mesh->mNormals[i];

			const float sign = ComputeTangentSign(N, T, B);
			vertex.tangent = { T.x, T.y, T.z, sign };
		}
		else
			vertex.tangent = { 1.f, 0.f, 0.f, 1.f };
	}
}

void ModelImporter::ParseMeshes(ImportedData& outData)
{
	outData.meshes.reserve(scene->mNumMeshes);
	SkeletonInfo* pSkeleton = outData.skeleton.get();
	
	const _uint rootBoneIdx = (pSkeleton ? pSkeleton->rootBoneIdx : (_uint)-1);

	set<_uint> physicsBoneIndices;

	for (_uint meshIdx = 0; meshIdx < scene->mNumMeshes; ++meshIdx)
	{
		aiMesh* curMesh = scene->mMeshes[meshIdx];
		 
		auto meshData = make_unique<MeshData>();
		meshData->name        = curMesh->mName.C_Str();
		meshData->materialIdx = curMesh->mMaterialIndex;
		meshData->type        = (meshData->name.contains("driverMesh")) ? MESHTYPE::Driver : MESHTYPE::Animated;
		
		const bool isSkinnedMesh = (curMesh->mNumBones > 0);
		meshData->layoutID       = isSkinnedMesh ? VertexLayoutID::PNUTanSkin : VertexLayoutID::PNUTan;
		
		if (isSkinnedMesh)
		{
			assert(pSkeleton && "ParseMeshes - Skinned mesh found but skeleton is null");
			ParseVerticesPNUTanSkin(curMesh, *meshData);

			vector<vector<pair<_uint, float>>> tempWeights;
			tempWeights.resize(curMesh->mNumVertices);
			ParseWeights(curMesh, *pSkeleton, tempWeights);

			vector<_uint> nonRootBoneIndicesInMesh;
			nonRootBoneIndicesInMesh.reserve(curMesh->mNumBones);

			for (_uint j = 0; j < curMesh->mNumBones; ++j)
			{
				const char* boneName = curMesh->mBones[j]->mName.C_Str();
				auto it = pSkeleton->boneNameToIdx.find(boneName);
				if (it != pSkeleton->boneNameToIdx.end())
				{
					const _uint boneIdx = it->second;
					if (boneIdx != rootBoneIdx)
						nonRootBoneIndicesInMesh.push_back(boneIdx);
				}
			}

			for (_uint v = 0; v < curMesh->mNumVertices; ++v)
			{
				auto& vw = tempWeights[v];

				const bool isLockedToRoot = (vw.size() == 1u && vw[0].first == rootBoneIdx);

				if (isLockedToRoot && !nonRootBoneIndicesInMesh.empty())
				{
					vw.clear();
					for (_uint boneIdx : nonRootBoneIndicesInMesh)
					{
						vw.push_back({ boneIdx, 1.f });
						physicsBoneIndices.insert(boneIdx);
					}
				}
				else
					erase_if(vw, [rootBoneIdx](const auto& p) {return p.first == rootBoneIdx; });

				sort(vw.begin(), vw.end(), [](const auto& a, const auto& b) {return a.second > b.second; });

				float total = 0.f;
				_uint count = 0u;
				for (const auto& w : vw)
				{
					if (count >= 4u) break;
					total += w.second;
					++count;
				}

				for (int k = 0; k < 4; ++k)
				{
					meshData->verticesPNUTanSkin[v].boneIndices[k] = 0;
					meshData->verticesPNUTanSkin[v].boneWeights[k] = 0.f;
				}

				count = 0u;
				for (const auto& w : vw)
				{
					if (count >= 4u) break;
					meshData->verticesPNUTanSkin[v].boneIndices[count] = w.first;
					meshData->verticesPNUTanSkin[v].boneWeights[count] = (total > 0.f) ? (w.second / total) : 0.f;
					++count;
				}

				const float w0 = meshData->verticesPNUTanSkin[v].boneWeights[0];
				const float w1 = meshData->verticesPNUTanSkin[v].boneWeights[1];
				const float w2 = meshData->verticesPNUTanSkin[v].boneWeights[2];
				const float w3 = meshData->verticesPNUTanSkin[v].boneWeights[3];
				if (w0 == 0.f && w1 == 0.f && w2 == 0.f && w3 == 0.f)
				{
					meshData->verticesPNUTanSkin[v].boneIndices[0] = 0;
					meshData->verticesPNUTanSkin[v].boneWeights[0] = 1.f;
				}
			}
		}
		else
			ParseVerticesPNUTan(curMesh, *meshData);

		ParseIndices(curMesh, *meshData);

		outData.meshes.push_back(move(meshData));
	}

	if (!physicsBoneIndices.empty() && outData.skeleton)
	{
		for (_uint boneIndex : physicsBoneIndices)
			outData.skeleton->bones[boneIndex].isAnimated = false;
	}
}

void ModelImporter::ParseWeights(const aiMesh* mesh, const SkeletonInfo& skeleton, vector<vector<pair<_uint, float>>>& outTempWeights)
{
	for (_uint i = 0; i < mesh->mNumBones; ++i)
	{
		aiBone* bone = mesh->mBones[i];
		string boneName = bone->mName.C_Str();

		auto it = skeleton.boneNameToIdx.find(boneName);
		if (it == skeleton.boneNameToIdx.end())
			continue;

		_uint boneIdx = it->second;

		for (_uint j = 0; j < bone->mNumWeights; ++j)
		{
			_uint vtxId = bone->mWeights[j].mVertexId;
			float weight = bone->mWeights[j].mWeight;
			outTempWeights[vtxId].push_back({ boneIdx, weight });
		}
	}
}

void ModelImporter::ParseIndices(const aiMesh* mesh, MeshData& outMesh)
{
	_uint numIndices = 0;
	for (_uint i = 0; i < mesh->mNumFaces; ++i)
		numIndices += mesh->mFaces[i].mNumIndices;

	outMesh.indices.reserve(numIndices);

	for (_uint i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		for (_uint j = 0; j < face.mNumIndices; ++j)
			outMesh.indices.push_back(face.mIndices[j]);
	}
}

float ModelImporter::ComputeTangentSign(const aiVector3D& normalAi, const aiVector3D& tangentAi, const aiVector3D& bitangentAi)
{
	// N, T, B (Assimp ±âÁØ) -> sign = sign( dot(cross(N,T), B))
	const _float3 normal    = { normalAi.x, normalAi.y, normalAi.z };
	const _float3 tangent   = { tangentAi.x, tangentAi.y, tangentAi.z };
	const _float3 biTangent = { bitangentAi.x, bitangentAi.y, bitangentAi.z };

	_vec N = XMLoadFloat3(&normal);
	_vec T = XMLoadFloat3(&tangent);
	_vec B = XMLoadFloat3(&biTangent);

	_vec crossNT = XMVector3Cross(N, T);
	float dotValue;
	XMStoreFloat(&dotValue, XMVector3Dot(crossNT, B));

	return (dotValue < 0.f) ? -1.f : +1.f;
}

void ModelImporter::ParseAnimations(ImportedData& outData)
{
	outData.animations.reserve(scene->mNumAnimations);

	for (_uint i = 0; i < scene->mNumAnimations; ++i)
	{
		const aiAnimation* anim = scene->mAnimations[i];
		auto clip = make_unique<AnimClip>();

		clip->name = anim->mName.C_Str();
		clip->duration = (float)anim->mDuration;
		clip->tickPerSec = (float)anim->mTicksPerSecond;
		if (clip->tickPerSec == 0.f)
			clip->tickPerSec = 30.f;

		for (_uint j = 0; j < anim->mNumChannels; ++j)
		{
			const aiNodeAnim* nodeAnim = anim->mChannels[j];
			string boneName = nodeAnim->mNodeName.C_Str();

			BoneAnim boneAnim;
			boneAnim.boneName = boneName;

			boneAnim.posKeys.reserve(nodeAnim->mNumPositionKeys);
			boneAnim.rotKeys.reserve(nodeAnim->mNumRotationKeys);
			boneAnim.scaleKeys.reserve(nodeAnim->mNumScalingKeys);

			for (_uint k = 0; k < nodeAnim->mNumPositionKeys; ++k)
			{
				Keyframe<_float3> key;
				key.time = (float)nodeAnim->mPositionKeys[k].mTime;
				const auto& val = nodeAnim->mPositionKeys[k].mValue;
				key.value = { val.x, val.y, val.z };
				boneAnim.posKeys.push_back(key);
			}

			for (_uint k = 0; k < nodeAnim->mNumRotationKeys; ++k)
			{
				Keyframe<_float4> key;
				key.time = (float)nodeAnim->mRotationKeys[k].mTime;
				const auto& val = nodeAnim->mRotationKeys[k].mValue;
				key.value = { val.x, val.y, val.z, val.w };
				boneAnim.rotKeys.push_back(key);
			}

			for (_uint k = 0; k < nodeAnim->mNumScalingKeys; ++k)
			{
				Keyframe<_float3> key;
				key.time = (float)nodeAnim->mScalingKeys[k].mTime;
				const auto& val = nodeAnim->mScalingKeys[k].mValue;
				key.value = { val.x, val.y, val.z };
				boneAnim.scaleKeys.push_back(key);
			}
			clip->boneAnims[boneAnim.boneName] = move(boneAnim);
		}
		outData.animations.push_back(move(clip));
	}
}

void ModelImporter::ParseMaterials(ImportedData& outData, const filesystem::path& fbxPath)
{
	for (_uint i = 0; i < scene->mNumMaterials; ++i)
	{
		aiMaterial* aiMtrl = scene->mMaterials[i];

		unique_ptr<MaterialData> materialData = TextureLoader::LoadMaterial(aiMtrl, scene, fbxPath);

		outData.materials.push_back(move(materialData));
	}
}