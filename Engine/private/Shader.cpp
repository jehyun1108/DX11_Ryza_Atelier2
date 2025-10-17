#include "Enginepch.h"

shared_ptr<Shader> Shader::CreateFromBlobs(const ShaderCreateDesc& desc)
{
	auto shader = make_shared<Shader>();
	if (FAILED(shader->Init(desc)))
		return nullptr;
	return shader;
}

HRESULT Shader::Init(const ShaderCreateDesc& desc)
{
	stages = desc.shaderTypes;
    assert(stages != SHADER::NONE && "No shader stage specified");

    if (stages & SHADER::VS) 
    {
        assert(desc.vsBlob);
        HR(DEVICE->CreateVertexShader(desc.vsBlob->GetBufferPointer(),desc.vsBlob->GetBufferSize(), nullptr, &vs));

        auto elements = desc.layout.Build();
        HR(DEVICE->CreateInputLayout(elements.data(), (UINT)elements.size(), desc.vsBlob->GetBufferPointer(), desc.vsBlob->GetBufferSize(), &inputLayout));
    }
    if (stages & SHADER::PS)
    {
        assert(desc.psBlob);
        HR(DEVICE->CreatePixelShader(desc.psBlob->GetBufferPointer(),desc.psBlob->GetBufferSize(), nullptr, &ps));
    }
    return S_OK;
}

void Shader::Bind(ID3D11DeviceContext* context)
{
    if (inputLayout)         context->IASetInputLayout(inputLayout.Get());

    if (stages & SHADER::VS) context->VSSetShader(vs.Get(), nullptr, 0);
    if (stages & SHADER::PS) context->PSSetShader(ps.Get(), nullptr, 0);
}