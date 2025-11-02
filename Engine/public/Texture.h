#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL Texture final
{
public:
	static shared_ptr<Texture> LoadFromFile(const wstring& fullPath, TextureColorSpace colorSpace);
	static shared_ptr<Texture> CreateSolidColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a, TextureColorSpace colorSpace);
	static shared_ptr<Texture> CreateSolidColorU32(uint32_t rgba, TextureColorSpace colorSpace);

	ID3D11ShaderResourceView* GetSrv() const { return srv.Get(); }
	DXGI_FORMAT        GetBaseFormat() const { return baseFormat; }
	TextureColorSpace  GetColorSpace() const { return colorSpace; }

	_uint GetWidth()  const { return width; }
	_uint GetHeight() const { return height; }

private:
	HRESULT InitFromFile(const wstring& fullPath, TextureColorSpace colorSpace);

private:
	ComPtr<ID3D11Resource>           resource;
	ComPtr<ID3D11ShaderResourceView> srv;

	DXGI_FORMAT       baseFormat = DXGI_FORMAT_UNKNOWN;
	TextureColorSpace colorSpace = TextureColorSpace::sRGB;

	_uint width  = 0;
	_uint height = 0;
};

NS_END

