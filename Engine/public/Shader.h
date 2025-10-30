#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL Shader final
{
public:
	Shader();
	static shared_ptr<Shader> CreateFromBlobs(const ShaderCreateDesc& desc);
	void Bind(ID3D11DeviceContext* context);

	bool hasTessellation() const { return (stages & SHADER::HS) && (stages & SHADER::DS); }

private:
	HRESULT Init(const ShaderCreateDesc& desc);

private:
	ID3D11Device* device{};
	SHADER stages = SHADER::NONE;

	ComPtr<ID3D11VertexShader> vs;
	ComPtr<ID3D11PixelShader>  ps;
	ComPtr<ID3D11HullShader>   hs;
	ComPtr<ID3D11DomainShader> ds;

	ComPtr<ID3D11InputLayout>  inputLayout;
};

NS_END