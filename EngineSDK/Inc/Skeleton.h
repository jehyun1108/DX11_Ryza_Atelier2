#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL Skeleton
{
public:
	explicit Skeleton(const SkeletonInfo& skeletonInfo);
	Skeleton(const Skeleton&) = delete;
	Skeleton& operator=(const Skeleton&) = delete;

public:
	Bone* rootBone{};
	vector<unique_ptr<Bone>> bones{};
	vector<Bone*> bonesByIdx{};
	map<string, _uint> boneNameToIdx{};
};

NS_END