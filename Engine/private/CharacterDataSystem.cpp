#include "Enginepch.h"

void CharacterDataSystem::BindEntity(EntityID entity, CharacterID character)
{
	characterByEntity[entity] = character;

	auto& buckets = entitiesByCharacters[character];
	buckets.push_back(entity);

	if (entityByCharacter.find(character) == entityByCharacter.end())
		entityByCharacter[character] = entity;
}

void CharacterDataSystem::UnBindEntity(EntityID entity)
{
	auto it = characterByEntity.find(entity);
	if (it != characterByEntity.end())
	{
		const CharacterID character = it->second;

		auto itBucket = entitiesByCharacters.find(character);
		if (itBucket != entitiesByCharacters.end())
		{
			auto& vec = itBucket->second;
			vec.erase(remove(vec.begin(), vec.end(), entity), vec.end());
			if (vec.empty())
			{
				entitiesByCharacters.erase(itBucket);
				entityByCharacter.erase(character);
			}
			else
			{
				if (entityByCharacter[character] == entity)
					entityByCharacter[character] = vec.front();
			}
		}
		characterByEntity.erase(it);
	}
	paramsByEntity.erase(entity);
}

CharacterID CharacterDataSystem::GetCharacterID(EntityID entity) const
{
	auto it = characterByEntity.find(entity);
	return (it != characterByEntity.end()) ? it->second : CharacterID::Unknown;
}

EntityID CharacterDataSystem::GetEntityID(CharacterID characterId) const
{
	auto it = entityByCharacter.find(characterId);
	return (it != entityByCharacter.end()) ? it->second : invalidEntity;
}

const vector<EntityID>& CharacterDataSystem::GetEntities(CharacterID character) const
{
	static const vector<EntityID> empty{};
	auto it = entitiesByCharacters.find(character);
	return (it != entitiesByCharacters.end()) ? it->second : empty;
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
	const CharacterID characterId = GetCharacterID(entity);
	if (characterId == CharacterID::Unknown) return L"";

	const auto& animData = registry.Get<AnimDataSystem>();
	return animData.GetClipName(characterId, context, key);
}

const ActionAnimSpec* CharacterDataSystem::GetActionSpec(EntityID entity) const
{
	const CharacterID characterId = GetCharacterID(entity);
	if (characterId == CharacterID::Unknown)
		return nullptr;

	const auto& actionRegistry = registry.Get<ActionAnimRegistry>();
	return actionRegistry.TryGet(characterId);
}

bool CharacterDataSystem::TryGetParams(EntityID entity, CharacterParams& outParams) const
{
	auto it = paramsByEntity.find(entity);
	if (it == paramsByEntity.end()) return false;
	
	outParams = it->second;
	return true;
}

CharacterParams CharacterDataSystem::GetParams(EntityID entity) const
{
	auto it = paramsByEntity.find(entity);
	if (it == paramsByEntity.end())
		return CharacterParams{};
	return it->second;
}