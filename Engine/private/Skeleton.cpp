#include "Enginepch.h"

Skeleton::Skeleton(const SkeletonInfo& skeletonInfo)
{
	const _uint boneCount = ENUM(skeletonInfo.bones.size());
	bones.reserve(boneCount);
	bonesByIdx.resize(boneCount);

	boneNameToIdx = skeletonInfo.boneNameToIdx;

	for (_uint i = 0; i < boneCount; ++i)
	{
		const auto& boneInfo = skeletonInfo.bones[i];
		bones.push_back(make_unique<Bone>(boneInfo.boneName, boneInfo.invBindPose, boneInfo.bindLocal, nullptr, i, boneInfo.isAnimated));
		bonesByIdx[i] = bones.back().get();
	}

	for (_uint i = 0; i < boneCount; ++i)
	{
		const int parentIdx = skeletonInfo.bones[i].parentIdx;
		if (parentIdx >= 0)
		{
			bonesByIdx[i]->parent = bonesByIdx[parentIdx];
			bonesByIdx[parentIdx]->children.push_back(bonesByIdx[i]);
		}
	}

	if (skeletonInfo.rootBoneIdx >= 0 && skeletonInfo.rootBoneIdx < (int)boneCount)
		rootBone = bonesByIdx[skeletonInfo.rootBoneIdx];
}
