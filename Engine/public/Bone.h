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

	_float4x4 animatedLocalTransform{}; // 부모 기준의 상대적인 움직임 (재귀적 계산을 위한 중간 값)
	_float4x4 combinedTransform{}; // 모델 기준의 최종적인 움직임 (스키닝에 사용될 '최종 결과물')
};

NS_END