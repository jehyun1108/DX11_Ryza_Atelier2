#pragma once

#include "ActionCamData.h"

NS_BEGIN(Engine)

class ENGINE_DLL ActionCamRegistry : public ISystem
{
public:
	explicit ActionCamRegistry(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;
	// BattleCommand
	void             RegisterCommandCam(const ActionDef& def) { defs[def.actionId] = def; }
	const ActionDef* FindCommand(ActionId id) const;
	TrackID          PlayActionCam(ActionId action, EntityID attacker, EntityID victim);
	// Character + Skill
	void                  RegisterSkillCam(CharacterID character, SpecialAnimTag tag, const ActionCamSpec& spec, const vector<ShotClip>& clips);
	const ActionCamEntry* FindSkill(CharacterID character, SpecialAnimTag tag) const;
	TrackID               PlaySkillCam(CharacterID character, SpecialAnimTag tag, EntityID attacker, EntityID victim);
	void                  StopTrack(TrackID id);

private:
	AnchorBinding     BuildAnchor(const ActionCamSpec& spec, EntityID attacker, EntityID victim) const;
	TrackSpawnRequest BuildTrackRequest(const ActionCamSpec& spec, EntityID attacker, EntityID victim) const;

private:
	unordered_map<ActionId, ActionDef> defs;
	unordered_map<ActionCamKey, ActionCamEntry, ActionCamKeyHasher> skillCams;

private:
	SystemRegistry&        registry;
	CamRegistry*           camReg{};
	BattleFormationSystem* formationSys{};
	CharacterDataSystem*   dataSys{};
};

NS_END