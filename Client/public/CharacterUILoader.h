#pragma once

NS_BEGIN(Client)

class CharacterUILoader
{
public:
	inline static void RegisterCharacterUI(CharacterDataSystem* dataSys)
	{
		{ // Ryza
			CharacterSpec spec{};
			spec.team             = BattleTeam::Ally;
			spec.party.baseLevel  = 1;
			spec.party.baseMaxHp  = 1200;
			spec.party.attackDmg  = 10;
			spec.party.baseMaxExp = 10;
			spec.ui.slotToTexKey[UITextureSlot::BattleCharacterIcon] = L"ryza_battleui";
			spec.ui.slotToTexKey[UITextureSlot::FatalDrive]          = L"ryza_fataldrive";
			spec.ui.slotToTexKey[UITextureSlot::TimelineIcon]        = L"timeline_ryza";
			dataSys->RegisterSpec(CharacterID::Ryza, spec);
		}
		{ // Klaudia
			CharacterSpec spec{};
			spec.team             = BattleTeam::Ally;
			spec.party.baseLevel  = 1;
			spec.party.baseMaxHp  = 1000;
			spec.party.attackDmg  = 12;
			spec.party.baseMaxExp = 10;
			spec.ui.slotToTexKey[UITextureSlot::BattleCharacterIcon] = L"klaudia_battleui";
			spec.ui.slotToTexKey[UITextureSlot::FatalDrive]          = L"klaudia_fataldrive";
			spec.ui.slotToTexKey[UITextureSlot::TimelineIcon]        = L"timeline_klaudia";
			dataSys->RegisterSpec(CharacterID::Klaudia, spec);
		}
		{ // Patricia
			CharacterSpec spec{}; 
			spec.team             = BattleTeam::Ally;
			spec.party.baseLevel  = 1;
			spec.party.baseMaxHp  = 800;
			spec.party.attackDmg  = 15;
			spec.party.baseMaxExp = 10;
			spec.ui.slotToTexKey[UITextureSlot::BattleCharacterIcon] = L"patricia_battleui";
			spec.ui.slotToTexKey[UITextureSlot::FatalDrive]          = L"patricia_fataldrive";
			spec.ui.slotToTexKey[UITextureSlot::TimelineIcon]        = L"timeline_patricia";
			dataSys->RegisterSpec(CharacterID::Patricia, spec);
		}
		{ // Angel
			CharacterSpec spec{};
			spec.team            = BattleTeam::Enemy;
			spec.enemy.attackDmg = 10;
			spec.enemy.baseMapHp = 600;
			spec.ui.slotToTexKey[UITextureSlot::TimelineIcon] = L"timeline_npc";
			dataSys->RegisterSpec(CharacterID::Angel, spec);
		}
	}
};


NS_END