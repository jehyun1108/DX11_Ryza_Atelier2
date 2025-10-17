#pragma once

NS_BEGIN(Engine)

struct ENGINE_DLL SocketData
{
	Handle childTf;
	Handle parentAnim;
	_uint  boneIdx = (_uint)-1;

	_float4x4 offsetMat{};
	_float3   offsetPos{};
	_float3   offsetRot{};
	bool      offsetDirty = true;
};

NS_END