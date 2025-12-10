#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL Material final
{
public:
	Material();
	explicit Material(const MaterialMeta& meta) { SetMeta(meta); }
	shared_ptr<Material> Clone() const;

	void SetMeta(const MaterialMeta& meta);
	const MaterialMeta& GetMeta() const { return meta; }

	void SetShaderKey(const wstring& key) { meta.shaderKey = key; }
	const wstring& GetShaderKey() const   { return meta.shaderKey; }

	void SetTextureKey(TEXSLOT slot, const wstring& key, SHADER stage = SHADER::PS);
	void SetSampler(TEXSLOT slot, SAMPLER sampler);

	void Resolve(ShaderCache& shaderCache, TextureCache& texCache);
	void Bind(ID3D11DeviceContext* context);

	MaterialBlend GetBlend() const     { return meta.blend; }
	void SetBlend(MaterialBlend blend) { meta.blend = blend; }

	bool IsTransparent()  const { return meta.blend != MaterialBlend::Opaque; }
	bool HasTesellation() const { return shader ? shader->hasTessellation() : false; }

private:
	void ComputeBeginEnd(_uint mask, _uint& begin, _uint& end);
	void ForEachRange(_uint mask, const function<void(_uint begin, _uint end)>& func);

private:
	SystemRegistry& registry = GAME.GetRegistry();
	MaterialMeta meta{};
	
	shared_ptr<Shader> shader;
	array<shared_ptr<Texture>, NUM_TEXSLOTS> textures;
	array<SHADER,  NUM_TEXSLOTS> stages;
	array<SAMPLER, NUM_TEXSLOTS> samplers;

	array<bool, NUM_TEXSLOTS> dirtyTexture{};
	array<bool, NUM_TEXSLOTS> dirtySampler{};

	array<_uint, NUM_SHADERSTAGES> usedMasks;
	array<_uint, NUM_SHADERSTAGES> prevUsedMasks;
	array<_uint, NUM_SHADERSTAGES> dirtyMasks;
};

NS_END