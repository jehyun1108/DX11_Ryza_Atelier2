#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL Bone 
{
public:
	Bone(const string& name, const _float4x4& invBindPose, const _float4x4& bindLocal, Bone* parent, _uint idx, bool isAnimated);
	void Update(const _fmat& parentMat);

public:
	bool isAnimated = false;
	string name{};
	_uint idx;
	Bone* parent;
	vector<Bone*> children{};

	_float4x4 invBindPose{};
	_float4x4 bindLocal{};

	_float4x4 animatedLocalTransform{}; // �θ� ������ ������� ������ (����� ����� ���� �߰� ��)
	_float4x4 combinedTransform{}; // �� ������ �������� ������ (��Ű�׿� ���� '���� �����')
};

NS_END