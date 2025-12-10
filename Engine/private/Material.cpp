#include "Enginepch.h"

static constexpr SHADER IdxToStage(_uint stageIdx)
{
	switch (static_cast<ShaderStageIdx>(stageIdx))
	{
	case ShaderStageIdx::VS: return SHADER::VS;
	case ShaderStageIdx::PS: return SHADER::PS;
	}
	return SHADER::NONE;
}

Material::Material()
{
	textures.fill(nullptr);
	stages.fill(SHADER::NONE);
	samplers.fill(SAMPLER::LINEAR);
	dirtyTexture.fill(false);
	dirtySampler.fill(false);

	usedMasks.fill(0);
	prevUsedMasks.fill(0);
	dirtyMasks.fill(0);
}

shared_ptr<Material> Material::Clone() const
{
	auto material = make_shared<Material>();
	material->SetMeta(this->meta);
	return material;
}

void Material::SetMeta(const MaterialMeta& meta)
{
	this->meta = meta;
	dirtyTexture.fill(true);
	dirtySampler.fill(true);

	usedMasks.fill(0);
	prevUsedMasks.fill(0);
	dirtyMasks.fill(0);
}

void Material::SetTextureKey(TEXSLOT slot, const wstring& key, SHADER stage)
{
	const _uint i     = ENUM(slot);
	if (i >= NUM_TEXSLOTS) return;
	meta.texKey[i]    = key;
	meta.stageMask[i] = stage;
	dirtyTexture[i]   = true;
}

void Material::SetSampler(TEXSLOT slot, SAMPLER sampler)
{
	const _uint i       = ENUM(slot);
	if (i >= NUM_TEXSLOTS) return;
	meta.samplerType[i] = sampler;
	dirtySampler[i]     = true;
}

void Material::Resolve(ShaderCache& shaderCache, TextureCache& texCache)
{
	{
		auto newShader = meta.shaderKey.empty() ? nullptr : shaderCache.Ensure(meta.shaderKey);
		if (newShader != shader)
			shader     = move(newShader);
	}
	prevUsedMasks = usedMasks;
	usedMasks.fill(0);
	dirtyMasks.fill(0);

	for (_uint i = 0; i < NUM_TEXSLOTS; ++i)
	{
		SHADER stage = meta.stageMask[i];
		if (stage == SHADER::NONE)
			stage = SHADER::PS;

		if (stages[i] != stage)
		{
			stages[i] = stage;
			dirtyTexture[i] = true;
		}

		if (samplers[i] != meta.samplerType[i])
		{
			samplers[i] = meta.samplerType[i];
			dirtySampler[i] = true;
		}

		shared_ptr<Texture> newTexture = meta.texKey[i].empty() ? nullptr : texCache.Ensure(meta.texKey[i]);
		if (textures[i] != newTexture)
		{
			textures[i] = move(newTexture);
			dirtyTexture[i] = true;
		}

		if (textures[i])
		{
			for (_uint stageIdx = 0; stageIdx < NUM_SHADERSTAGES; ++stageIdx)
			{
				const SHADER stageBit = IdxToStage(stageIdx);

				if (stages[i] & stageBit)
				{
					usedMasks[stageIdx] |= (1u << i);
					if (dirtyTexture[i])
						dirtyMasks[stageIdx] |= (1u << i);
				}
			}
		}
	}
}

void Material::Bind(ID3D11DeviceContext* context)
{
	if (shader)
		shader->Bind(context);

	static ID3D11ShaderResourceView* const nullSRVs[NUM_TEXSLOTS] = {};

	auto clearRanges = [&](ShaderStageIdx stageIdx, _uint mask)
		{
			if (mask == 0) return;
			ForEachRange(mask, [&](uint32_t begin, uint32_t end)
				{
					const _uint count = end - begin;
					switch (stageIdx)
					{
					case ShaderStageIdx::VS: context->VSSetShaderResources(begin, count, nullSRVs + begin); break;
					case ShaderStageIdx::PS: context->PSSetShaderResources(begin, count, nullSRVs + begin); break;
					}
				});
		};

	for (_uint i = 0; i < NUM_SHADERSTAGES; ++i)
	{
		const _uint toClear = prevUsedMasks[i] & ~usedMasks[i];
		clearRanges(static_cast<ShaderStageIdx>(i), toClear);
	}

	static thread_local array<ID3D11ShaderResourceView*, NUM_TEXSLOTS> srvs{};

	auto bindDirtyRange = [&](ShaderStageIdx stageIdx, _uint dirtyMask, _uint usedMask)
		{
			const _uint target = dirtyMask & usedMask;
			if (target == 0) return;

			const SHADER stageBit = IdxToStage(ENUM(stageIdx));

			ForEachRange(target, [&](uint32_t begin, uint32_t end)
				{
					for (_uint j = begin; j < end; ++j)
						srvs[j] = textures[j] ? textures[j]->GetSrv() : nullptr;

					const _uint count = end - begin;
					switch (stageIdx)
					{
					case ShaderStageIdx::VS: context->VSSetShaderResources(begin, count, srvs.data() + begin); break;
					case ShaderStageIdx::PS: context->PSSetShaderResources(begin, count, srvs.data() + begin); break;
					}

					for (_uint j = begin; j < end; ++j)
						if (stages[j] & stageBit) dirtyTexture[j] = false;
				});
		};

	for (_uint i = 0; i < NUM_SHADERSTAGES; ++i)
		bindDirtyRange(static_cast<ShaderStageIdx>(i), dirtyMasks[i], usedMasks[i]);

	for (_uint i = 0; i < NUM_TEXSLOTS; ++i)
	{
		if (!dirtySampler[i]) continue;
		if (textures[i]) 
			registry.Get<Renderer>().BindSamplers(stages[i], static_cast<TEXSLOT>(i), samplers[i]);
		dirtySampler[i] = false;
	}
	prevUsedMasks = usedMasks;
}

void Material::ComputeBeginEnd(_uint mask, _uint& begin, _uint& end)
{
	if (mask == 0)
	{
		begin = NUM_TEXSLOTS;
		end = 0;
		return;
	}
	_uint ubegin = 0;
	_uint uend = NUM_TEXSLOTS;

	while (ubegin < NUM_TEXSLOTS && ((mask >> ubegin) & 1u) == 0) ++ubegin;
	while (uend > 0 && ((mask >> (uend - 1)) & 1u) == 0)          --uend;

	begin = ubegin; 
	end   = uend;
}

void Material::ForEachRange(_uint mask, const function<void(_uint begin, _uint end)>& func)
{
	_uint i = 0;
	while (i < NUM_TEXSLOTS)
	{
		while (i < NUM_TEXSLOTS && ((mask & (1u << i)) == 0)) ++i;
		if (i >= NUM_TEXSLOTS) break;

		_uint begin = i;
		while (i < NUM_TEXSLOTS && ((mask & (1u << i)) != 0)) ++i;
		_uint end = i;
		func(begin, end);
	}
}