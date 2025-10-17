#pragma once

namespace Engine
{
	struct EngineDesc
	{
		HWND hWnd;
		WINMODE winMode;
		_uint WinX, WinY;
		_uint levelCount;
	};

	struct TransformDesc
	{
		optional<float> speed;
		optional<float> rotSpeed;
		optional<_float3> pos;
		optional<_float3> scale;
		optional<_float4> rot;
	};

	struct UIDesc
	{
		float posX = 0.f;
		float posY = 0.f;

		float sizeX = 0.f;
		float sizeY = 0.f;

		// anchor & pivot Ãß°¡ 
	};

	struct ObjDesc
	{
		_uint levelID;
		LAYER layerType{};
		wstring name;
		wstring modelKey;

		optional<TransformDesc> tfDesc;
	};
}