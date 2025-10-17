#include "Enginepch.h"

Material::Material()
{
	textures.fill(nullptr);
	stages.fill(SHADER::NONE);
	samplers.fill(SAMPLER::LINEAR);
	dirtyTexture.fill(false);
	dirtySampler.fill(false);
	usedMaskVS = usedMaskPS = 0;
}

void Material::SetDesc(const MaterialDesc& desc)
{
	this->desc = desc;
	dirtyTexture.fill(true);
	dirtySampler.fill(true);
	usedMaskVS = usedMaskPS = 0;    
}

void Material::SetShaderKey(const wstring& key)
{
	desc.shaderKey = key;
}

const wstring& Material::GetShaderKey() const
{
	return desc.shaderKey;
}

void Material::SetTextureKey(TEXSLOT slot, const wstring& key, SHADER stage)
{
	const _uint i     = ENUM(slot);
	if (i >= NUM_TEXSLOTS) return;
	desc.texKey[i]    = key;
	desc.stageMask[i] = stage;
	dirtyTexture[i]   = true;
}

void Material::SetSampler(TEXSLOT slot, SAMPLER sampler)
{
	const _uint i       = ENUM(slot);
	if (i >= NUM_TEXSLOTS) return;
	desc.samplerType[i] = sampler;
	dirtySampler[i]     = true;
}

void Material::Resolve(ShaderCache& shaderCache, TextureCache& texCache)
{
	// Shader
	{
		auto newShader = desc.shaderKey.empty() ? nullptr : shaderCache.Ensure(desc.shaderKey);
		if (newShader != shader)
			shader      = move(newShader);
	}
	// 이전 Mask 보존 
	prevUsedMaskVS = usedMaskVS;
	prevUsedMaskPS = usedMaskPS;

	usedMaskVS  = usedMaskPS  = 0;
	dirtyMaskVS = dirtyMaskPS = 0;

	for (_uint i = 0; i < NUM_TEXSLOTS; ++i)
	{
		// Shader
		SHADER stage = desc.stageMask[i];
		if (stage == SHADER::NONE)
			stage = SHADER::PS;

		if (stages[i] != stage)
		{
			stages[i] = stage;
			dirtyTexture[i] = true;
		}

		// Sampler
		if (samplers[i] != desc.samplerType[i])
		{
			samplers[i] = desc.samplerType[i];
			dirtySampler[i] = true;
		}

		// Texture
		shared_ptr<Texture> newTexture = desc.texKey[i].empty() ? nullptr : texCache.Ensure(desc.texKey[i]);
		if (textures[i] != newTexture)
		{
			textures[i] = move(newTexture);
			dirtyTexture[i] = true;
		}

		// Mask / DirtyMask
		if (textures[i])
		{
			if (stages[i] & SHADER::VS)
			{
				usedMaskVS |= (1u << i);
				if (dirtyTexture[i])
					dirtyMaskVS |= (1u << i);
			}
			if (stages[i] & SHADER::PS)
			{
				usedMaskPS |= (1u << i);
				if (dirtyTexture[i])
					dirtyMaskPS |= (1u << i);
			}
		}
	}
}

void Material::Bind(ID3D11DeviceContext* context)
{
	// Shader
	if (shader)
		shader->Bind(context);

	// 지난 프레임에 쓰이고 이번프레임에 안쓰이는 슬롯 null 로 정리
	const _uint toClearVS = prevUsedMaskVS & ~usedMaskVS;
	const _uint toClearPS = prevUsedMaskPS & ~usedMaskPS;

	static ID3D11ShaderResourceView* const nullSRVs[NUM_TEXSLOTS] = {};

	auto clearRanges = [&](SHADER stage, _uint mask)
		{
			if (mask == 0) return;
			ForEachRange(mask, [&](uint32_t begin, uint32_t end)
				{
					const _uint count = end - begin;
					if (stage & SHADER::VS) context->VSSetShaderResources(begin, count, nullSRVs + begin);
					if (stage & SHADER::PS) context->PSSetShaderResources(begin, count, nullSRVs + begin);
				});
		};

	clearRanges(SHADER::VS, toClearVS);
	clearRanges(SHADER::PS, toClearPS);

	// 이번 프레임에 필요한 슬롯 / 변경된 슬롯
	static thread_local array<ID3D11ShaderResourceView*, NUM_TEXSLOTS> srvs{};

	auto bindDirtyRange = [&](_uint dirtyMask, _uint usedMask, SHADER stage)
		{
			const _uint target = dirtyMask & usedMask;
			if (target == 0) return;

			ForEachRange(target, [&](uint32_t begin, uint32_t end)
				{
					for (_uint j = begin; j < end; ++j)
						srvs[j] = textures[j] ? textures[j]->GetSrv() : nullptr;

					const _uint count = end - begin;
					if (stage & SHADER::VS) context->VSSetShaderResources(begin, count, srvs.data() + begin);
					if (stage & SHADER::PS) context->PSSetShaderResources(begin, count, srvs.data() + begin);

					for (_uint j = begin; j < end; ++j)
						if (stages[j] & stage) dirtyTexture[j] = false;
				});
		};

	bindDirtyRange(dirtyMaskVS, usedMaskVS, SHADER::VS);
	bindDirtyRange(dirtyMaskPS, usedMaskPS, SHADER::PS);

	// Sampler
	for (_uint i = 0; i < NUM_TEXSLOTS; ++i)
	{
		if (!dirtySampler[i]) continue;
		if (textures[i]) 
			GAME.BindSamplers(stages[i], static_cast<TEXSLOT>(i), samplers[i]);
		dirtySampler[i] = false;
	}

	prevUsedMaskVS = usedMaskVS;
	prevUsedMaskPS = usedMaskPS;
}

void Material::UnBind(ID3D11DeviceContext* context)
{
	static ID3D11ShaderResourceView* const kNullSRVs[NUM_TEXSLOTS] = {};

	auto doClear = [&](SHADER stage, _uint mask)
		{
			if (mask == 0) return;
			ForEachRange(mask, [&](uint32_t begin, uint32_t end)
				{
					const _uint count = end - begin;
					if (stage & SHADER::VS) context->VSSetShaderResources(begin, count, kNullSRVs + begin);
					if (stage & SHADER::PS) context->PSSetShaderResources(begin, count, kNullSRVs + begin);
				});
		};

	doClear(SHADER::VS, usedMaskVS);
	doClear(SHADER::PS, usedMaskPS);

	prevUsedMaskVS = 0;
	prevUsedMaskPS = 0;
}

shared_ptr<Material> Material::Clone() const
{
	auto material = make_shared<Material>();
	material->SetDesc(this->desc);
	return material;
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
