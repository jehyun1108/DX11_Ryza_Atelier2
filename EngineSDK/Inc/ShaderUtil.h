#pragma once

#include "InputLayoutBuilder.h"

NS_BEGIN(Engine)

struct ShaderMeta
{
	SHADER shaderTypes = SHADER::NONE;
	InputLayoutBuilder layout;
	wstring vsCsoPath;
	wstring psCsoPath;
	wstring hsCsoPath;
	wstring dsCsoPath;
};

struct ShaderCreateDesc
{
	SHADER shaderTypes = SHADER::NONE;
	InputLayoutBuilder layout;
	ComPtr<ID3DBlob> vsBlob;
	ComPtr<ID3DBlob> psBlob;
	ComPtr<ID3DBlob> hsBlob;
	ComPtr<ID3DBlob> dsBlob;
};

NS_END