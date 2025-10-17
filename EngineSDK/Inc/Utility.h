#pragma once

#include "GuiUtility.h"
#include "GpuUtil.h"
#include "JsonUtil.h"
#include "TextureUtil.h"
#include "ShaderUtil.h"
#include "ModelUtil.h"

#include "TransformData.h"
#include "BinaryUtil.h"

NS_BEGIN(Engine)

class ENGINE_DLL Utility final
{
public:
	static _uint ToDxBool(bool b) { return b ? TRUE : FALSE; }
	// -------------- string - wstring -----------------------------

	// UTF-16 & UTF-8
	static wstring ToWString(const string& str);
	static string ToString(const wstring& wstr);

	// --------------- Enum -> String --------------------------------
	static string ToString(CBUFFERSLOT slot);
	static string ToString(TEXSLOT slot);
	static string ToString(SAMPLER type);
	static string ToString(SHADER stage);
	static string ToString(LAYER layer);

	// -------------- 난수 생성 --------------------------
	static int Range(int min, int max);
	static float Range(float min, float max);

	// -------------- ImGui -------------------------------
	static optional<filesystem::path> OpenFbxFileDialog();
	static wstring EnsureDoubleNullFilter(const wstring& rawFilter);
	
	static optional<filesystem::path> OpenFileDialog(const wstring& filter, const wstring& defaultExtension = L"", const filesystem::path& initDir = {});
	static optional<filesystem::path> SaveFileDialog(const wstring& filter, const wstring& defaultFileName, const wstring& defaultExtension = L"", const filesystem::path& initDir = {});

	// ostream 객체 (outFile)에 count 개수만큼 탭 문자를 출력하는 함수
	static void Indent(ostream& os, int count);

	static void ReadString(ifstream& ifs, string& outStr);

	template<typename T>
	static void ReadData(ifstream& ifs, T& data)
	{
		static_assert(is_trivially_copyable_v<T>, "type T must be trivially copyable for binary read");

		ifs.read(reinterpret_cast<char*>(&data), sizeof(T));
	}

	template<typename T>
	static void ReadVector(ifstream& ifs, vector<T>& vec)
	{
		static_assert(is_trivially_copyable_v<T>, "Vector type T must be trivially copyable for binary read");

		_uint size{};
		ifs.read(reinterpret_cast<char*>(&size), sizeof(_uint));
		if (size > 0)
		{
			vec.resize(size);
			ifs.read(reinterpret_cast<char*>(vec.data()), sizeof(T) * size);
		}
		else
			vec.clear();
	}	

	template<typename...Args>
	static void Log(const wstring_view format_str, Args&&...args)
	{
		wstring formatted_string = vformat(format_str, make_wformat_args(args...));
		OutputDebugStringW(formatted_string.c_str());
		OutputDebugStringW(L"\n");
	}

	static _uint AlignTo16Bytes(_uint value) { return (value + 15u) & ~15u; }

	static float EaseCubic(float t)
	{
		return (t < 0.5f) ? 4.f * t * t * t : 1.f - powf(-2.f * t + 2.f, 3.f) / 2.f;
	}

	static float Saturate(float x)
	{
		return max(0.f, min(1.f, x));
	}

	static float EaseCosine(float t)
	{
		return 0.5f * (1.f - cosf(3.141592f * t));
	}

	static float Jitter(float base, float ratio)
	{
		const float random = Range(-1.f, 1.f);
		return max(1e-4f, base * (1.f + ratio * random));
	}

	static float Jitter(float base)
	{
		return Jitter(base, 0.15f);
	}

	static DXGI_FORMAT ToSRGB(DXGI_FORMAT format);
	static DXGI_FORMAT ToLinearUNORM(DXGI_FORMAT format);
	static HRESULT CreateSRVWithPerceptualFlag(ID3D11Resource* resource, DXGI_FORMAT baseFormat, bool perceptualSRGB, ID3D11ShaderResourceView** OutSRV);
	static DXGI_FORMAT GetResourceFormat(ID3D11Resource* resource);
	static TextureColorSpace SlotToColorSpace(TEXSLOT slot);

	// --------------------------------------
	static _vec Right() { return XMVectorSet(1.f, 0.f, 0.f, 0.f); }
	static _vec Up()    { return XMVectorSet(0.f, 1.f, 0.f, 0.f); }
	static _vec Look()  { return XMVectorSet(0.f, 0.f, 1.f, 0.f); }

	static _float3 Right3() { return _float3(1.f, 0.f, 0.f); }
	static _float3 Up3()    { return _float3(0.f, 1.f, 0.f); }
	static _float3 Look3()  { return _float3(0.f, 0.f, 1.f); }

	static _float3 ToEuler(const _float4& quat);
	static _float3 ToEuler(_fvec vQuat);
	static string  ToLower(string str);
	static wstring ToLower(wstring wstr);

	static wstring Normalize(const wstring& in);
	static wstring MakeModelKey(const filesystem::path& modelPath);
	static pair<const wchar_t*, TextureColorSpace> GetDefaultTex(TEXSLOT slot);

	static bool NearlyEqual(float a, float b, float epsilon = 1e-6f) { return fabsf(a - b) <= epsilon; }
	static float Length(const _float3& v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }
	
	// ---------------- BoundingBox
	static BoundingBox ToAABBFromOBB(const BoundingOrientedBox& obb);
	static pair<_float3, float> ToSphereFromAABB(const BoundingBox& aabb);
	static BoundingBox ToAABBFromSphere(const _float3& center, float radius);
	static BoundingOrientedBox ToOBBFromAABB(const BoundingBox& aabb);

	static _mat MakeWorldMat(const TransformData& tf);
};

NS_END