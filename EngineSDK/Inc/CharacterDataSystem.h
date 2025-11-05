#pragma once

#include "CharacterData.h"

NS_BEGIN(Engine)

class ENGINE_DLL CharacterDataSystem : public ISystem
{
public:
	explicit CharacterDataSystem(SystemRegistry& registry) : registry(registry) {}

	void        BindEntity(EntityID entity, CharacterID character);
	void        UnBindEntity(EntityID entity);
	CharacterID GetCharacterID(EntityID entity) const;

	EntityID                GetEntityID(CharacterID characterId) const;
	const vector<EntityID>& GetEntities(CharacterID character) const;

	AnimProfile           ResolveProfile(EntityID entity, AnimContext context) const;
	const wstring&        GetClipName(EntityID entity, AnimContext context, AnimKey key) const;
	const ActionAnimSpec* GetActionSpec(EntityID entity) const;

	void            SetParams(EntityID entity, const CharacterParams& params) { paramsByEntity[entity] = params; }
	bool            TryGetParams(EntityID entity, CharacterParams& outParams) const;
	CharacterParams GetParams(EntityID entity) const;

	// UI Textures
	void                       RegisterUITextures(CharacterID characterId, const CharacterUITextures& textures);
	const CharacterUITextures* FindUITexturesByCharacter(CharacterID characterId) const;
	const wstring*             TryGetTextureKey(EntityID entity, UITextureSlot texSlot) const;
	const CharacterUITextures* FindUITexturesByEntity(EntityID entity) const;

private:
	SystemRegistry& registry;

	unordered_map<EntityID, CharacterID>         characterByEntity;
	unordered_map<CharacterID, vector<EntityID>> entitiesByCharacters;
	unordered_map<CharacterID, EntityID>         entityByCharacter;
	unordered_map<EntityID, CharacterParams>     paramsByEntity;
	
	unordered_map<CharacterID, CharacterUITextures> uiTexturesByCharacter;
};

NS_END
