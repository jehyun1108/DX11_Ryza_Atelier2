#include "Enginepch.h"
#include "Utility.h"

static constexpr DWORD openFlags = OFN_PATHMUSTEXIST   | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR;
static constexpr DWORD saveFlags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR;

wstring Utility::ToWString(const string& str)
{
	if (str.empty()) return L"";

	const int requiredChars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str.data(), static_cast<int>(str.size()), nullptr, 0);
	if (requiredChars <= 0) return L"";
	
	wstring wstr(static_cast<size_t>(requiredChars), L'\0');
	const int written = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str.data(), static_cast<int>(str.size()), wstr.data(), requiredChars);
	if (written != requiredChars) return L"";
	return wstr;
}

string Utility::ToString(const wstring& wstr)
{
	if (wstr.empty()) return "";

	const int requiredBytes = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wstr.data(), 
		static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);
	if (requiredBytes <= 0) return "";

	string str(static_cast<size_t>(requiredBytes), '\0');
	const int written = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wstr.data(), static_cast<int>(wstr.size()), str.data(), requiredBytes, nullptr, nullptr);
	if (written != requiredBytes) return "";
	return str;
}

string Utility::ToString(CBUFFERSLOT slot)
{
	switch (slot)
	{
	case CBUFFERSLOT::CAMERA:  return "Camera";
	case CBUFFERSLOT::OBJ:     return "Obj";
	case CBUFFERSLOT::LIGHT:   return "Light";
	default:                   return "Unknown_CBufferSlot";
	}
}

string Utility::ToString(TEXSLOT slot)
{
	switch (slot)
	{
	case TEXSLOT::ALBEDO:	  return "Albedo";
	case TEXSLOT::NORMAL:	  return "Normal";
	case TEXSLOT::ROUGHNESS: return "Roughness";
	case TEXSLOT::METALIC:   return "Metalic";
	default:                      return "UnKnown_TextureSlot";
	}
}

string Utility::ToString(SAMPLER type)
{
	switch (type)
	{
	case SAMPLER::POINT:        return "Point";
	case SAMPLER::LINEAR:       return "Linear";
	case SAMPLER::ANISOTROPIC:	return "Anisotropic";
	case SAMPLER::SHADOW:		return "Shadow";
	default:                    return "Unknown_SamplerType";
	}
}
string Utility::ToString(SHADER stage)
{
	if (stage == SHADER::NONE) return "NONE";
	string result;

	if (stage & SHADER::VS) result += "VS ";
	if (stage & SHADER::PS) result += "PS ";
	if (stage & SHADER::GS) result += "GS ";
	if (stage & SHADER::HS) result += "HS ";
	if (stage & SHADER::DS) result += "DS ";
	if (stage & SHADER::CS) result += "CS ";

	if (!result.empty())
		result.pop_back();

	return result;
}

string Utility::ToString(LAYER layer)
{
	switch (layer)
	{
	case LAYER::CAMERA:	  return "Camera";
	case LAYER::SOCKET:   return "Socket";
	case LAYER::TERRAIN:  return "Terrain";
	case LAYER::PLAYER:	  return "Player";
	case LAYER::MONSTER:  return "Monster";
	case LAYER::EFFECT:	  return "Effect";
	case LAYER::UI:		  return "UI";
	case LAYER::MAPOBJ:   return "MapObj";
	default:              return "Unknown_Layer";    
	}
}

int Utility::Range(int min, int max)
{
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> dis(min, max);
	return dis(gen);
}

float Utility::Range(float min, float max)
{
	random_device rd;
	mt19937 gen(rd());
	uniform_real_distribution<float> dis(min, max);
	return dis(gen);
}

optional<filesystem::path> Utility::OpenFbxFileDialog()
{
	wchar_t szFilePath[MAX_PATH] = L"";

	OPENFILENAMEW ofn = {};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = g_hWnd;
	ofn.lpstrFile = szFilePath;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = L"3D Model Files (*.fbx, *.gltf, *.glb)\0*.fbx;*.gltf;*.glb\0"
		L"FBX Files (*.fbx)\0*.fbx\0"
		L"glTF Files (*.gltf, *.glb)\0*.gltf;*.glb\0"
		L"All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileNameW(&ofn))
		return filesystem::path(szFilePath);

	return nullopt;
}

wstring Utility::EnsureDoubleNullFilter(const wstring& rawFilter)
{
	if (rawFilter.empty()) return L"\0\0"s;

	wstring filtered = rawFilter;
	if (filtered.back() != L'\0')
		filtered.push_back(L'\0');
	filtered.push_back(L'\0');
	return filtered;
}

optional<filesystem::path> Utility::OpenFileDialog(const wstring& filter, const wstring& defaultExtension, const filesystem::path& initDir)
{
	wstring fileBuffer;
	fileBuffer.resize(MAX_PATH * 4, L'\0');

	OPENFILENAMEW ofn{};
	ofn.lStructSize           = sizeof(ofn);
	ofn.hwndOwner             = nullptr;
	const wstring fixedFilter = EnsureDoubleNullFilter(filter);
	ofn.lpstrFilter           = fixedFilter.c_str();
	ofn.nFilterIndex          = 1;
	ofn.lpstrFile             = fileBuffer.data();
	ofn.nMaxFile              = static_cast<DWORD>(fileBuffer.size());
	ofn.Flags                 = openFlags;

	wstring defaultExt = defaultExtension;
	if (!defaultExt.empty() && defaultExt.front() == L'.')
		defaultExt.erase(defaultExt.begin());
	ofn.lpstrDefExt = defaultExt.empty() ? nullptr : defaultExt.c_str();

	wstring wInitDir = initDir.empty() ? wstring() : initDir.wstring();
	ofn.lpstrInitialDir = wInitDir.empty() ? nullptr : wInitDir.c_str();

	if (GetOpenFileNameW(&ofn))
		return filesystem::path(ofn.lpstrFile);
	return nullopt;
}

optional<filesystem::path> Utility::SaveFileDialog(const wstring& filter, const wstring& defaultFileName, const wstring& defaultExtension, const filesystem::path& initDir)
{
	wstring fileBuffer = defaultFileName;
	if (fileBuffer.empty())
		fileBuffer = L"untitled";

	fileBuffer.resize(MAX_PATH * 4, L'\0');

	OPENFILENAMEW ofn{};
	ofn.lStructSize           = sizeof(ofn);
	ofn.hwndOwner             = nullptr;
	const wstring fixedFilter = EnsureDoubleNullFilter(filter);
	ofn.lpstrFilter           = fixedFilter.c_str();
	ofn.nFilterIndex          = 1;
	ofn.lpstrFile             = fileBuffer.data();
	ofn.nMaxFile              = static_cast<DWORD>(fileBuffer.size());
	ofn.Flags                 = saveFlags;

	wstring defaultExt = defaultExtension;
	if (!defaultExt.empty() && defaultExt.front() == L'.')
		defaultExt.erase(defaultExt.begin());
	ofn.lpstrDefExt = defaultExt.empty() ? nullptr : defaultExt.c_str();

	wstring wInitDir    = initDir.empty() ? wstring() : initDir.wstring();
	ofn.lpstrInitialDir = wInitDir.empty() ? nullptr : wInitDir.c_str();

	if (GetSaveFileNameW(&ofn))
		return filesystem::path(ofn.lpstrFile);
	return nullopt;
}

void Utility::Indent(ostream& os, int count)
{
	for (int i = 0; i < count; ++i)
		os << "\t"; // 탭 문자 '\t' 출력
}

void Utility::ReadString(ifstream& ifs, string& outStr)
{
	_uint len;
	ifs.read(reinterpret_cast<char*>(&len), sizeof(_uint));
	outStr.resize(len);
	ifs.read(&outStr[0], len);
}

DXGI_FORMAT Utility::ToSRGB(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM:        return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	case DXGI_FORMAT_B8G8R8A8_UNORM:        return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	case DXGI_FORMAT_B8G8R8X8_UNORM:        return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
	case DXGI_FORMAT_BC1_UNORM:             return DXGI_FORMAT_BC1_UNORM_SRGB;
	case DXGI_FORMAT_BC2_UNORM:             return DXGI_FORMAT_BC2_UNORM_SRGB;
	case DXGI_FORMAT_BC3_UNORM:             return DXGI_FORMAT_BC3_UNORM_SRGB;
	case DXGI_FORMAT_BC7_UNORM:             return DXGI_FORMAT_BC7_UNORM_SRGB;
	default:                                return format; // sRGB 변형이 없으면 그대로
	}
}

DXGI_FORMAT Utility::ToLinearUNORM(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:   return DXGI_FORMAT_R8G8B8A8_UNORM;
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:   return DXGI_FORMAT_B8G8R8A8_UNORM;
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:   return DXGI_FORMAT_B8G8R8X8_UNORM;
	case DXGI_FORMAT_BC1_UNORM_SRGB:        return DXGI_FORMAT_BC1_UNORM;
	case DXGI_FORMAT_BC2_UNORM_SRGB:        return DXGI_FORMAT_BC2_UNORM;
	case DXGI_FORMAT_BC3_UNORM_SRGB:        return DXGI_FORMAT_BC3_UNORM;
	case DXGI_FORMAT_BC7_UNORM_SRGB:        return DXGI_FORMAT_BC7_UNORM;
	default:                                return format;
	}
}

HRESULT Utility::CreateSRVWithPerceptualFlag(ID3D11Resource* resource, DXGI_FORMAT baseFormat, bool perceptualSRGB, ID3D11ShaderResourceView** outSRV)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = perceptualSRGB ? ToSRGB(baseFormat) : ToLinearUNORM(baseFormat);

	// 리소스 타입에 맞춰 ViewDimension 채우기 (2D 기준)
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = -1; // 모든 Mip
	srvDesc.Texture2D.MostDetailedMip = 0;

	GameInstance& game = GameInstance::GetInstance();

	return game.GetDevice()->CreateShaderResourceView(resource, &srvDesc, outSRV);
}

DXGI_FORMAT Utility::GetResourceFormat(ID3D11Resource* resource)
{
	if (!resource) return DXGI_FORMAT_UNKNOWN;

	D3D11_RESOURCE_DIMENSION dimension{};
	resource->GetType(&dimension);

	if (dimension == D3D11_RESOURCE_DIMENSION_TEXTURE2D)
	{
		ComPtr<ID3D11Texture2D> tex2D;
		resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(tex2D.GetAddressOf()));
		if (tex2D)
		{
			D3D11_TEXTURE2D_DESC desc{};
			tex2D->GetDesc(&desc);
			return desc.Format;
		}
	}
	else if (dimension == D3D11_RESOURCE_DIMENSION_TEXTURE3D)
	{
		ComPtr<ID3D11Texture3D> tex3D;
		resource->QueryInterface(__uuidof(ID3D11Texture3D), reinterpret_cast<void**>(tex3D.GetAddressOf()));
		if (tex3D)
		{
			D3D11_TEXTURE3D_DESC desc{};
			tex3D->GetDesc(&desc);
			return desc.Format;
		}
	}

	return DXGI_FORMAT_UNKNOWN;
}

TextureColorSpace Utility::SlotToColorSpace(TEXSLOT slot)
{

	switch (slot)
	{
	case TEXSLOT::ALBEDO:
	case TEXSLOT::EMISSIVE:
		return TextureColorSpace::sRGB;

	case TEXSLOT::NORMAL:
	case TEXSLOT::ROUGHNESS:
	case TEXSLOT::AO:
		return TextureColorSpace::Linear;

	default:
		return TextureColorSpace::sRGB;
	}
}

_float3 Utility::ToEuler(const _float4& quat)
{
	const _mat mRot = XMMatrixRotationQuaternion(XMLoadFloat4(&quat));

	const float pitch = asinf(-mRot.r[2].m128_f32[1]);                                 
	const float yaw   = atan2f(mRot.r[2].m128_f32[0], mRot.r[2].m128_f32[2]);       
	const float roll  = atan2f(mRot.r[0].m128_f32[1], mRot.r[1].m128_f32[1]);

	return _float3(XMConvertToDegrees(pitch), XMConvertToDegrees(yaw), XMConvertToDegrees(roll));
}

_float3 Utility::ToEuler(_fvec vQuat)
{
	_float4 quat{};
	XMStoreFloat4(&quat, vQuat);
	return ToEuler(quat);
}

string Utility::ToLower(string str)
{
	transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}

wstring Utility::ToLower(wstring wstr)
{
	transform(wstr.begin(), wstr.end(), wstr.begin(), ::tolower);
	return wstr;
}

wstring Utility::Normalize(const wstring& in)
{
	wstring out = in;
	//  백슬래시->슬래시
	replace(out.begin(), out.end(), L'\\', L'/');

	transform(out.begin(), out.end(), out.begin(),
		[](wchar_t c) { return (wchar_t)towlower(c); });

	// 연속 슬래시 정리
	wstring norm;
	norm.reserve(out.size());
	bool lastSlash = false;
	for (wchar_t ch : out)
	{
		if (ch == L'/')
		{
			if (!lastSlash)
				norm.push_back(ch);
			lastSlash = true;
		}
		else
		{
			norm.push_back(ch);
			lastSlash = false;
		}
	}
	// 앞/뒤 공백 제거
	auto l = norm.find_first_not_of(L" \t\r\n");
	auto r = norm.find_last_not_of(L" \t\r\n");
	if (l == wstring::npos) return L"";
	return norm.substr(l, r - l + 1);
}

wstring Utility::MakeModelKey(const filesystem::path& modelPath)
{
	const auto rootAbs = filesystem::absolute(PathMgr::GetModelPath());
	filesystem::path rel = filesystem::relative(modelPath, rootAbs);

	rel.replace_extension();
	return L"models/" + Utility::Normalize(rel.generic_wstring());
}

pair<const wchar_t*, TextureColorSpace> Utility::GetDefaultTex(TEXSLOT slot)
{
	switch (slot)
	{
	// sRGB 
	case TEXSLOT::ALBEDO:    return { L"builtin/white", TextureColorSpace::sRGB };
	case TEXSLOT::EMISSIVE:  return { L"builtin/black", TextureColorSpace::sRGB };

	// Linear
	case TEXSLOT::NORMAL:    return { L"builtin/flat_normal", TextureColorSpace::Linear }; 
	case TEXSLOT::ROUGHNESS: return { L"builtin/one_linear", TextureColorSpace::Linear }; 
	case TEXSLOT::METALIC:   return { L"builtin/zero_linear", TextureColorSpace::Linear }; 
	case TEXSLOT::AO:        return { L"builtin/one_linear", TextureColorSpace::Linear }; 

	default:                 return { L"builtin/white", TextureColorSpace::sRGB };
	}
}

BoundingBox Utility::ToAABBFromOBB(const BoundingOrientedBox& obb)
{
	_float3 corners[8];
	obb.GetCorners(corners);
	_float3 minCorner = corners[0];
	_float3 maxCorner = corners[0];
	for (int i = 1; i < 8; ++i)
	{
		minCorner.x = min(minCorner.x, corners[i].x);
		minCorner.y = min(minCorner.y, corners[i].y);
		minCorner.z = min(minCorner.z, corners[i].z);
		maxCorner.x = max(maxCorner.x, corners[i].x);
		maxCorner.y = max(maxCorner.y, corners[i].y);
		maxCorner.z = max(maxCorner.z, corners[i].z);
	}
	_float3 center{ (minCorner.x + maxCorner.x) * 0.5f, (minCorner.y + maxCorner.y) * 0.5f, (minCorner.z + maxCorner.z) * 0.5f };
	_float3 extent{ (maxCorner.x - minCorner.x) * 0.5f, (maxCorner.y - minCorner.y) * 0.5f, (maxCorner.z - minCorner.z) * 0.5f };
	return BoundingBox(center, extent);
}

pair<_float3, float> Utility::ToSphereFromAABB(const BoundingBox& aabb)
{
	const _float3 extent = aabb.Extents;
	const float radius = max(extent.x, max(extent.y, extent.z)); // <= 더 작아짐
	return { aabb.Center, max(radius, 1e-6f) };
}

BoundingBox Utility::ToAABBFromSphere(const _float3& center, float radius)
{
	_float3 extent{ radius, radius, radius };
	return BoundingBox(center, extent);
}

BoundingOrientedBox Utility::ToOBBFromAABB(const BoundingBox& aabb)
{
	BoundingOrientedBox obb{};
	obb.Center = aabb.Center;
	obb.Extents = aabb.Extents;
	obb.Orientation = _float4{ 0, 0, 0, 1 }; 
	return obb;
}

_mat Utility::MakeWorldMat(const TransformData& tf)
{
	const _vec vPos   = XMLoadFloat3(&tf.pos);
	const _vec vScale = XMLoadFloat3(&tf.scale);
	const _vec vRot   = XMLoadFloat4(&tf.rot);

	const _mat mScale = XMMatrixScalingFromVector(vScale);
	const _mat mRot   = XMMatrixRotationQuaternion(vRot);
	const _mat mTrans = XMMatrixTranslationFromVector(vPos);
	return mScale * mRot * mTrans;
}
