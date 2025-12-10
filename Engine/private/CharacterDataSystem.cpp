#include "Enginepch.h"

void CharacterDataSystem::OnBoot()
{
	animRegistry = &registry.Get<ActionAnimRegistry>();
	animDataSys  = &registry.Get<AnimDataSystem>();
}

void CharacterDataSystem::BindEntity(EntityID entity, CharacterID id)
{
	assert(entity != invalidEntity);
	characterByEntity[entity] = id;
	auto& bucket = entitiesByCharacters[id];
	bucket.push_back(entity);
	if (!entityByCharacter.contains(id))
		entityByCharacter[id] = entity;
}

void CharacterDataSystem::UnBindEntity(EntityID entity)
{
	auto it = characterByEntity.find(entity);
	if (it == characterByEntity.end()) return;

	const CharacterID cid = it->second;
	auto bit = entitiesByCharacters.find(cid);
	if (bit != entitiesByCharacters.end())
	{
		auto& v = bit->second;
		v.erase(remove(v.begin(), v.end(), entity), v.end());
		if (v.empty())
		{
			entitiesByCharacters.erase(bit);
			entityByCharacter.erase(cid);
		}
		else
		{
			if (entityByCharacter[cid] == entity)
				entityByCharacter[cid] = v.front();
		}
	}
	characterByEntity.erase(it);
}

CharacterID CharacterDataSystem::GetCharacterID(EntityID entity) const
{
	auto it = characterByEntity.find(entity);
	assert(it != characterByEntity.end());
	return it->second;
}

EntityID CharacterDataSystem::GetEntityID(CharacterID id) const
{
	auto it = entityByCharacter.find(id);
	assert(it != entityByCharacter.end());
	return it->second;
}

const vector<EntityID>& CharacterDataSystem::GetEntities(CharacterID id) const
{
	auto it = entitiesByCharacters.find(id);
	assert(it != entitiesByCharacters.end());
	return it->second;
}

AnimProfile CharacterDataSystem::ResolveProfile(EntityID entity, AnimContext context) const
{
	AnimProfile profile{};
	profile.character = GetCharacterID(entity);
	profile.context = context;
	return profile;
}

const wstring& CharacterDataSystem::GetClipName(EntityID entity, AnimContext context, AnimKey key) const
{
	const CharacterID id = GetCharacterID(entity);
	return animDataSys->GetClipName(id, context, key);
}

const ActionAnimSpec& CharacterDataSystem::GetActionSpec(EntityID entity) const
{
	const CharacterID id = GetCharacterID(entity);
	return animRegistry->Get(id);
}

void CharacterDataSystem::RegisterSpec(CharacterID id, const CharacterSpec& spec)
{
	specByCharacter[id] = spec;
	if (!spec.ui.slotToTexKey.empty())
		uiTexturesByCharacter.erase(id);
}

const CharacterSpec& CharacterDataSystem::GetSpec(CharacterID id) const
{
	auto it = specByCharacter.find(id);
	assert(it != specByCharacter.end());
	return it->second;
}

BattleTeam CharacterDataSystem::GetTeam(EntityID entity) const
{
	const CharacterID id = GetCharacterID(entity);
	auto it = specByCharacter.find(id);
	assert(it != specByCharacter.end());
	return it->second.team;
}

const CharacterUITextures& CharacterDataSystem::GetUITexturesByCharacter(CharacterID id) const
{
	if (auto sit = specByCharacter.find(id); sit != specByCharacter.end())
	{
		if (!sit->second.ui.slotToTexKey.empty())
			return sit->second.ui;
	}
	auto it = uiTexturesByCharacter.find(id);
	assert(it != uiTexturesByCharacter.end());
	return it->second;
}

const wstring& CharacterDataSystem::GetTextureKey(EntityID entity, UITextureSlot slot) const
{
	const CharacterID id = GetCharacterID(entity);

	if (auto sit = specByCharacter.find(id); sit != specByCharacter.end())
	{
		if (auto it = sit->second.ui.slotToTexKey.find(slot);
			it != sit->second.ui.slotToTexKey.end())
			return it->second;
	}
	const auto& tex = GetUITexturesByCharacter(id);
	auto jt = tex.slotToTexKey.find(slot);
	assert(jt != tex.slotToTexKey.end());
	return jt->second;
}

void CharacterDataSystem::SetCamAnchor(CharacterID id, EntityID anchor)
{
	auto it = specByCharacter.find(id);
	assert(it != specByCharacter.end()); 
	it->second.camAnchorEntity = anchor;
}

EntityID CharacterDataSystem::GetCamAnchor(CharacterID id) const
{
	return GetSpec(id).camAnchorEntity;
}

EntityID CharacterDataSystem::GetCamAnchor(EntityID entity) const
{
	const CharacterID ch = GetCharacterID(entity);
	return GetCamAnchor(ch);
}
