#pragma once

NS_BEGIN(Engine)

// DTO: Data Transfer Object -> 데이터 전달용 구조체 (파일, 엔진 경계에서 쓰는 순수 데이터 묶음)

struct ENGINE_DLL WorldInstanceDto
{
	string logicalKey; // UTF-8
	float  pos[3];
	float  rotQuat[4];
	float  scale[3];
	_uint  layerMask;
};

NS_END