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
	case LAYER::SKYBOX:   return "Skybox";
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

_float2 Utility::Clamp2D(_float2 vecXY, float maxLength)
{
	const float lenSq = vecXY.x * vecXY.x + vecXY.y * vecXY.y;
	if (lenSq <= maxLength * maxLength) return vecXY;

	const float len = sqrtf(lenSq);
	if (len <= 1e-6f) return _float2(0.f, 0.f);

	const float scale = maxLength / len;
	return _float2{ vecXY.x * scale, vecXY.y * scale };
}

_float2 Utility::Normalize(_float2 v)
{
	const float lengthSq = v.x * v.x + v.y * v.y;
	if (lengthSq <= 1e-12f) return _float2{ 0.f, 0.f };
	const float invLen = 1.0f / sqrtf(lengthSq);
	return _float2{ v.x * invLen, v.y * invLen };
}

void Utility::WrapToTwoPi(float& radians)
{
	static constexpr float twoPi = 6.28318530717958647692f;
	if (radians > twoPi || radians < 0.f)
	{
		radians = fmodf(radians, twoPi);
		if (radians < 0.f) radians += twoPi;
	}
}

float Utility::ExtractYawFromWorld(const _float4x4& mat)
{
	_mat M = XMLoadFloat4x4(&mat);
	_vec forward = M.r[2];
	const float fx = XMVectorGetX(forward);
	const float fz = XMVectorGetZ(forward);
	return atan2f(fx, fz); 
}

_mat Utility::RemoveScaleKeepRotTrans(_mat M)
{
	XMVECTOR S, R, T;
	if (!XMMatrixDecompose(&S, &R, &T, M)) return M;
	const XMVECTOR one = XMVectorSet(1.f, 1.f, 1.f, 0.f);
	return XMMatrixAffineTransformation(one, XMVectorZero(), R, T);
}

string Utility::StrPathStem(const wstring& wstr)
{
	if (wstr.empty()) return {};
	filesystem::path pathObj(wstr);
	wstring wStem = pathObj.stem().wstring();
	return ToString(wStem);
}

SkyRule Utility::GetSkyRuleByIdx(_uint submeshIdx)
{
	switch (submeshIdx)
	{
	case 0: // 배경
	case 1: // 배경
		return { SkyQueue::Opaque, SkyCull::Back, false, false };

	case 2: // 달
		return { SkyQueue::Opaque, SkyCull::Back, false, false };

	case 3: // 중앙 띠(운하 같은 판넬) 경계 보이면 AlphaBlend + Cull None 권장
		return { SkyQueue::Alpha, SkyCull::None, true, false };
    
    case 4: // 구름
    	return { SkyQueue::Alpha, SkyCull::Front, true, false };

	default:
		return { SkyQueue::Opaque, SkyCull::Front, false, false };
	}
}

bool Utility::FileExists(const filesystem::path& candidatePath)
{
	error_code ioError;
	return filesystem::exists(candidatePath, ioError) && filesystem::is_regular_file(candidatePath, ioError);
}

filesystem::path Utility::MakeNormalMapPath(const filesystem::path& diffuseFullPath)
{
	const filesystem::path& parentFolder = diffuseFullPath.parent_path();
	const wstring extension = diffuseFullPath.extension().wstring();

	const wstring stem = diffuseFullPath.stem().wstring();
	if (stem.empty()) return {};

	if (!all_of(stem.begin(), stem.end(), [](wchar_t ch) {return ch >= L'0' && ch <= L'9'; })) return {};

	const size_t digitCount = stem.size();

	try
	{
		long long numericValue = stoll(stem);
		numericValue += 1;

		wstringstream formatted;
		formatted << setw(static_cast<int>(digitCount)) << setfill(L'0') << numericValue;
		const wstring incrementedStem = formatted.str();

		filesystem::path sibling = parentFolder / (incrementedStem + extension);
		return sibling;
	}
	catch(...)
	{
		return {};
	}
}

_vec Utility::BuildLookRot(const _vec& forward, const _vec& worldUp)
{
	_vec z = XMVector3Normalize(forward);
	_vec x = XMVector3Normalize(XMVector3Cross(worldUp, z));
	_vec y = XMVector3Normalize(XMVector3Cross(z, x));

	_mat rot = XMMatrixIdentity();
	rot.r[0] = x; rot.r[1] = y; rot.r[2] = z;
	return XMQuaternionRotationMatrix(rot);
}

float Utility::WrapToPi(float rad)
{
	rad = fmodf(rad, XM_2PI);

	while (rad > XM_PI)
		rad -= XM_2PI;

	while (rad < -XM_PI)
		rad += XM_2PI;

	return rad;
}

_uint Utility::ComputeIdxStride(DXGI_FORMAT fmt)
{
	switch (fmt)
	{
	case DXGI_FORMAT_R16_UINT: return 2;
	case DXGI_FORMAT_R32_UINT: return 4;
	default:                   return 0;
	}
}

float Utility::Clamp(float v, float lo, float hi)
{
	return (v < lo) ? lo : (v > hi ? hi : v);
}

float Utility::SignedAngRad2D(const _float2& a, const _float2& b)
{
	const float dot = a.x * b.x + a.y * b.y;
	const float det = a.x * b.y - a.y * b.x; 
	return atan2f(det, dot);
}

_float2 Utility::Rotate2D(const _float2& v, float ang)
{
	const float c = cosf(ang);
	const float s = sinf(ang);
	return _float2{ v.x * c - v.y * s, v.x * s + v.y * c };
}

_float2 Utility::Normalize(float x, float z)
{
	const float len = sqrtf(x * x + z * z);
	if (len < 1e-6f) return _float2{ 0.f, -1.f };
	return _float2{ x / len, z / len };
}

_float3 Utility::NormalizeXZ(const _float3& v)
{
	const float len = sqrtf(v.x * v.x + v.z * v.z);
	if (len <= 1e-6f) return _float3{ 0.f, 0.f, 0.f };
	const float inv = 1.f / len;
	return _float3{ v.x * inv, 0.f, v.z * inv };
}

_float2 Utility::Normalize(const _float3& v)
{
	const float len = sqrtf(v.x * v.x + v.z * v.z);
	if (len <= 1e-6f) return _float2{ 0.f, 0.f };
	const float inv = 1.f / len;
	return _float2{ v.x * inv, v.z * inv };
}

_float3 Utility::Normalize3(const _float2& xz)
{
	return _float3{ xz.x, 0.f, xz.y };
}
