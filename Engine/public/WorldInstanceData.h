#pragma once

NS_BEGIN(Engine)

// DTO: Data Transfer Object -> ������ ���޿� ����ü (����, ���� ��迡�� ���� ���� ������ ����)

struct ENGINE_DLL WorldInstanceDto
{
	string logicalKey; // UTF-8
	float  pos[3];
	float  rotQuat[4];
	float  scale[3];
	_uint  layerMask;
};

NS_END