#include "Enginepch.h"

Shader::Shader()
{
    device = DEVICE;
}

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
        HR(device->CreateVertexShader(desc.vsBlob->GetBufferPointer(),desc.vsBlob->GetBufferSize(), nullptr, &vs));

        auto elements = desc.layout.Build();
        HR(device->CreateInputLayout(elements.data(), (UINT)elements.size(), desc.vsBlob->GetBufferPointer(), desc.vsBlob->GetBufferSize(), &inputLayout));
    }
    if (stages & SHADER::PS)
    {
        assert(desc.psBlob);
        HR(device->CreatePixelShader(desc.psBlob->GetBufferPointer(),desc.psBlob->GetBufferSize(), nullptr, &ps));
    }
    if (stages & SHADER::HS)
    {
        assert(desc.hsBlob);
        HR(device->CreateHullShader(desc.hsBlob->GetBufferPointer(), desc.hsBlob->GetBufferSize(), nullptr, &hs));
    }
    if (stages & SHADER::DS)
    {
        assert(desc.dsBlob);
        HR(DEVICE->CreateDomainShader(desc.dsBlob->GetBufferPointer(), desc.dsBlob->GetBufferSize(), nullptr, &ds));
    }
    return S_OK;
}

void Shader::Bind(ID3D11DeviceContext* context)
{
    if (inputLayout)        
        context->IASetInputLayout(inputLayout.Get());

    context->VSSetShader((stages & SHADER::VS) ? vs.Get() : nullptr, nullptr, 0);
    context->PSSetShader((stages & SHADER::PS) ? ps.Get() : nullptr, nullptr, 0);
    context->HSSetShader((stages & SHADER::HS) ? hs.Get() : nullptr, nullptr, 0);
    context->DSSetShader((stages & SHADER::DS) ? ds.Get() : nullptr, nullptr, 0);
}