#include "Enginepch.h"

Bone::Bone(const string& name, const _float4x4& invBindPose, const _float4x4& bindLocal, Bone* parent, _uint idx, bool isAnimated)
	:name(name), invBindPose(invBindPose), bindLocal(bindLocal), parent(parent), idx(idx), isAnimated(isAnimated)
{
	XMStoreFloat4x4(&animatedLocalTransform, XMMatrixIdentity());
	XMStoreFloat4x4(&combinedTransform, XMMatrixIdentity());
}

// (FK: Forward Kinematics)
void Bone::Update(const _fmat& parentMat)
{
	_mat localMat = XMLoadFloat4x4(&animatedLocalTransform);
	_mat combinedMat = localMat * parentMat;

	XMStoreFloat4x4(&combinedTransform, combinedMat);
	
	for (Bone* child : children)
		child->Update(combinedMat);
}