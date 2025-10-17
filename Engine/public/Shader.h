#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL Shader final
{
public:
	static shared_ptr<Shader> CreateFromBlobs(const ShaderCreateDesc& desc);
	void Bind(ID3D11DeviceContext* context);

private:
	HRESULT Init(const ShaderCreateDesc& desc);

private:
	SHADER stages = SHADER::NONE;
	ComPtr<ID3D11VertexShader> vs;
	ComPtr<ID3D11PixelShader>  ps;
	ComPtr<ID3D11InputLayout>  inputLayout;
};

NS_END