#pragma once

#include "InputLayoutBuilder.h"

NS_BEGIN(Engine)

struct ShaderMeta
{
	SHADER shaderTypes = SHADER::NONE;
	InputLayoutBuilder layout;
	wstring vsCsoPath;
	wstring psCsoPath;
};

struct ShaderCreateDesc
{
	SHADER shaderTypes = SHADER::NONE;
	InputLayoutBuilder layout;
	ComPtr<ID3DBlob> vsBlob;
	ComPtr<ID3DBlob> psBlob;
};

NS_END