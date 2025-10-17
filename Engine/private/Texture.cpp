#include "Enginepch.h"

// CreateTextureFromFileEx 로 flag 를 직접 설정하면서 세밀한 제어가능.

shared_ptr<Texture> Texture::LoadFromFile(const wstring& fullPath, TextureColorSpace colorSpace)
{
	auto instance = make_shared<Texture>();
	if (FAILED(instance->InitFromFile(fullPath, colorSpace))) 
		return nullptr;
	return instance;
}

shared_ptr<Texture> Texture::CreateSolidColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a, TextureColorSpace colorSpace)
{
	auto texture = make_shared<Texture>();
	texture->colorSpace = colorSpace;

	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = 1;
	desc.Height = 1;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc = { 1, 0 };
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	const uint8_t pixel[4] = { r, g, b, a };
	D3D11_SUBRESOURCE_DATA data{};
	data.pSysMem = pixel;
	data.SysMemPitch = 4;

	ComPtr<ID3D11Texture2D> tex2D;
	HR(DEVICE->CreateTexture2D(&desc, &data, tex2D.GetAddressOf()));

	texture->resource = tex2D;
	texture->baseFormat = desc.Format;

	HR(DEVICE->CreateShaderResourceView(texture->resource.Get(), nullptr, texture->srv.GetAddressOf()));
	return texture;
}

shared_ptr<Texture> Texture::CreateSolidColorU32(uint32_t rgba, TextureColorSpace colorSpace)
{
	uint8_t r = (rgba >> 0) & 0xFF;
	uint8_t g = (rgba >> 8) & 0xFF;
	uint8_t b = (rgba >> 16) & 0xFF;
	uint8_t a = (rgba >> 24) & 0xFF;
	return CreateSolidColor(r, g, b, a, colorSpace);
}

HRESULT Texture::InitFromFile(const wstring& fullPath, TextureColorSpace colorSpace)
{
	this->colorSpace = colorSpace;

	wstring ext = fullPath.substr(fullPath.find_last_of(L".") + 1);
	transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

	const bool perceptualSRGB = (colorSpace == TextureColorSpace::sRGB);
	HRESULT hr = E_FAIL;

	if (ext == L"dds")
	{
		DDS_LOADER_FLAGS ddsFlags = perceptualSRGB ? DDS_LOADER_FORCE_SRGB : DDS_LOADER_DEFAULT;

		hr = CreateDDSTextureFromFileEx(DEVICE, fullPath.c_str(), 0, D3D11_USAGE_DEFAULT, 
			D3D11_BIND_SHADER_RESOURCE, 0, 0, ddsFlags, resource.GetAddressOf(), nullptr);
	}
	else
	{
		WIC_LOADER_FLAGS wicFlags = perceptualSRGB ? WIC_LOADER_FORCE_SRGB : WIC_LOADER_IGNORE_SRGB;

		hr = CreateWICTextureFromFileEx(DEVICE, DC, fullPath.c_str(), 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, wicFlags, resource.GetAddressOf(), nullptr);
	}

	if (FAILED(hr))
	{
		assert(false && "Texture Loading failed!");
		return hr;
	}

	// 리소스 실제 포맷 기록 (디버그 정보용)
	baseFormat = Utility::GetResourceFormat(resource.Get());

	// SRV는 '리소스 포맷 그대로' 생성
	hr = DEVICE->CreateShaderResourceView(resource.Get(), nullptr, srv.GetAddressOf());
	if (FAILED(hr))
	{
		assert(false && "Texture SRV creation failed");
		return hr;
	}

	return S_OK;
}