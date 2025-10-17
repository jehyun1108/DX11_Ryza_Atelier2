#pragma once

#include "MaterialUtil.h"

NS_BEGIN(Engine)

class ENGINE_DLL Material final
{
public:
	Material();
	explicit Material(const MaterialDesc& desc) { SetDesc(desc); }

	void SetDesc(const MaterialDesc& desc);
	const MaterialDesc& GetDesc() const { return desc; }

	void SetShaderKey(const wstring& key);
	const wstring& GetShaderKey() const;

	void SetTextureKey(TEXSLOT slot, const wstring& key, SHADER stage = SHADER::PS);
	void SetSampler(TEXSLOT slot, SAMPLER sampler);

	void Resolve(ShaderCache& shaderCache, TextureCache& texCache);
	void Bind(ID3D11DeviceContext* context);
	void UnBind(ID3D11DeviceContext* context);

	MaterialBlend GetBlend() const     { return desc.blend; }
	void SetBlend(MaterialBlend blend) { desc.blend = blend; }

	bool IsTransparent() const { return desc.blend != MaterialBlend::Opaque; }
	shared_ptr<Material> Clone() const;

private:
	void ComputeBeginEnd(_uint mask, _uint& begin, _uint& end);
	void ForEachRange(_uint mask, const function<void(_uint begin, _uint end)>& func);

private:
	GameInstance& game = GameInstance::GetInstance();
	MaterialDesc desc{};
	
	shared_ptr<Shader> shader;
	array<shared_ptr<Texture>, NUM_TEXSLOTS> textures;
	array<SHADER,  NUM_TEXSLOTS> stages;
	array<SAMPLER, NUM_TEXSLOTS> samplers;

	array<bool, NUM_TEXSLOTS> dirtyTexture{};
	array<bool, NUM_TEXSLOTS> dirtySampler{};

	_uint usedMaskVS = 0;
	_uint usedMaskPS = 0;
	_uint prevUsedMaskVS = 0;
	_uint prevUsedMaskPS = 0;
	_uint dirtyMaskVS = 0;
	_uint dirtyMaskPS = 0;
};

NS_END