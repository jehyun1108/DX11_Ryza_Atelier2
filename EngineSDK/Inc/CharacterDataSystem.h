#pragma once

#include "CharacterData.h"

NS_BEGIN(Engine)

class ENGINE_DLL CharacterDataSystem : public ISystem
{
public:
	explicit CharacterDataSystem(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void                       BindEntity(EntityID entity, CharacterID id);
	void                       UnBindEntity(EntityID entity);
	CharacterID                GetCharacterID(EntityID entity) const;

	EntityID                   GetEntityID(CharacterID id) const;
	const vector<EntityID>&    GetEntities(CharacterID id) const;

	AnimProfile                ResolveProfile(EntityID entity, AnimContext context) const;
	const wstring&             GetClipName(EntityID entity, AnimContext context, AnimKey key) const;
	const ActionAnimSpec&      GetActionSpec(EntityID entity) const;

	void                       RegisterSpec(CharacterID id,  const CharacterSpec& spec);
	bool                       HasSpec(CharacterID id)       const { return specByCharacter.contains(id); }
	const CharacterSpec&       GetSpec(CharacterID id)       const;
	BattleTeam                 GetTeam(EntityID entity)      const;
	const PartySpec&           GetPartySpec(CharacterID id)  const { return GetSpec(id).party; }
	const EnemySpec&           GetEnemySpec(CharacterID id)  const { return GetSpec(id).enemy; }

	void                       RegisterUITextures(CharacterID id, const CharacterUITextures& textures) { uiTexturesByCharacter[id] = textures; }
	const CharacterUITextures& GetUITexturesByCharacter(CharacterID id) const;
	const CharacterUITextures& GetUITexturesByEntity(EntityID entity)   const { return GetUITexturesByCharacter(GetCharacterID(entity)); }
	const wstring&             GetTextureKey(EntityID entity, UITextureSlot slot) const;

	void                       SetCamAnchor(CharacterID id, EntityID anchor);
	EntityID                   GetCamAnchor(CharacterID id) const;
	EntityID                   GetCamAnchor(EntityID entity) const;

private:
	unordered_map<EntityID, CharacterID>            characterByEntity;
	unordered_map<CharacterID, vector<EntityID>>    entitiesByCharacters;
	unordered_map<CharacterID, EntityID>            entityByCharacter;
	unordered_map<CharacterID, CharacterUITextures> uiTexturesByCharacter;
	unordered_map<CharacterID, CharacterSpec>       specByCharacter;

private:
	SystemRegistry&     registry;
	ActionAnimRegistry* animRegistry{};
	AnimDataSystem*     animDataSys{};
};

NS_END
