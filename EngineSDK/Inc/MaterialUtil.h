#pragma once

NS_BEGIN(Engine)

enum class MaterialBlend : uint8_t {Opaque, Alphablend, Additive };

constexpr _uint NUM_TEXSLOTS = ENUM(TEXSLOT::END);

struct ENGINE_DLL MaterialDesc
{
	wstring shaderKey;
	array<wstring, NUM_TEXSLOTS> texKey{};
	array<SHADER,  NUM_TEXSLOTS> stageMask{};
	array<SAMPLER, NUM_TEXSLOTS> samplerType{};
	
	MaterialBlend blend = MaterialBlend::Opaque;
	bool alphaTest = false;
};

NS_END