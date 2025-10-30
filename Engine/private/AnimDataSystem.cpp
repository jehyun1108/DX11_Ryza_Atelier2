#include "Enginepch.h"

void AnimDataSystem::RegisterDefaultClips()
{
	// Ryza - Field
	ClipSet ryzaField{};
    ryzaField.nameByKey = {
        { AnimKey::Idle,       L"PC20A_00000" },
        { AnimKey::WalkStart,  L"PC20A_00020" },
        { AnimKey::WalkLoop,   L"PC20A_00021" },
        { AnimKey::WalkEnd,    L"PC20A_00022" },
        { AnimKey::JumpStart,  L"PC20A_00070" },
        { AnimKey::JumpLoop,   L"PC20A_00071" },
        { AnimKey::JumpEnd,    L"PC20A_00072" },
        { AnimKey::RunStart,   L"PC20A_00130" },
        { AnimKey::RunLoop,    L"PC20A_00131" },
        { AnimKey::RunEnd,     L"PC20A_00132" },
        { AnimKey::FieldSwing, L"PC20A_00200" },
    };
    Register(CharacterID::Ryza, AnimContext::Field, ryzaField);
    // Ryza - Battle
    ClipSet ryzaBattle{};
    ryzaBattle.nameByKey = {
        { AnimKey::Battle_Idle,             L"PC20A_01000" },
        { AnimKey::Battle_RunStart,         L"PC20A_01040" },
        { AnimKey::Battle_RunLoop,          L"PC20A_01041" },
        { AnimKey::Battle_RunEnd,           L"PC20A_01042" },
        { AnimKey::Battle_Celemony_1A,      L"PC20A_01050" },
        { AnimKey::Battle_Celemony_1B,      L"PC20A_01051" },
        { AnimKey::Battle_Celemony_2A,      L"PC20A_01060" },
        { AnimKey::Battle_Celemony_2B,      L"PC20A_01061" },
        { AnimKey::Battle_Attack_FinishedA, L"PC20A_01080" },
        { AnimKey::Battle_Attack_FinishedB, L"PC20A_01081" },
        { AnimKey::Battle_Attack_FinishedC, L"PC20A_01082" },
        { AnimKey::Battle_Skill_1A,         L"PC20A_01110" },
        { AnimKey::Battle_Skill_1B,         L"PC20A_01111" },
        { AnimKey::Battle_Skill_1C,         L"PC20A_01111" },
        { AnimKey::Battle_AttackA,          L"PC20A_01300" },
        { AnimKey::Battle_AttackB,          L"PC20A_01301" },
        { AnimKey::Battle_AttackC,          L"PC20A_01302" },
        { AnimKey::Battle_StartA,           L"PC20A_30130" },
        { AnimKey::Battle_StartB,           L"PC20A_30131" },
        { AnimKey::Battle_StartC,           L"PC20A_30132" },
        
    };
    Register(CharacterID::Ryza, AnimContext::Battle, ryzaBattle);
    // Kluadia Field
    ClipSet klaudiaField{};
    klaudiaField.nameByKey = {

    };
    Register(CharacterID::Kluadia, AnimContext::Field, klaudiaField);
    // Kluadia Battle
    ClipSet klaudiaBattle{};
    klaudiaBattle.nameByKey = {
        { AnimKey::Battle_Idle,             L"PC21A_01000" },
        { AnimKey::Battle_RunStart,         L"PC21A_01040" },
        { AnimKey::Battle_RunLoop,          L"PC21A_01041" },
        { AnimKey::Battle_RunEnd,           L"PC21A_01042" },
        { AnimKey::Battle_Celemony_1A,      L"PC21A_01050" },
        { AnimKey::Battle_Celemony_1B,      L"PC21A_01051" },
        { AnimKey::Battle_Celemony_2A,      L"PC21A_01060" },
        { AnimKey::Battle_Celemony_2B,      L"PC21A_01061" },
        { AnimKey::Battle_Attack_FinishedA, L"PC21A_01080" },
        { AnimKey::Battle_Attack_FinishedB, L"PC21A_01081" },
        { AnimKey::Battle_Attack_FinishedC, L"PC21A_01082" },
        { AnimKey::Battle_Skill_1A,         L"PC21A_01110" },
        { AnimKey::Battle_Skill_1B,         L"PC21A_01111" },
        { AnimKey::Battle_Skill_1C,         L"PC21A_01111" },
        { AnimKey::Battle_AttackA,          L"PC21A_01300" },
        { AnimKey::Battle_AttackB,          L"PC21A_01301" },
        { AnimKey::Battle_AttackC,          L"PC21A_01302" },
        { AnimKey::Battle_StartA,           L"PC21A_03030" },
        { AnimKey::Battle_StartB,           L"PC21A_03031" },
        { AnimKey::Battle_StartC,           L"PC21A_03032" },
    };
    Register(CharacterID::Kluadia, AnimContext::Battle, klaudiaBattle);
    // Patricia Field
    ClipSet patriciaField{};
    patriciaField.nameByKey = {

    };
    Register(CharacterID::Patricia, AnimContext::Field, patriciaField);
    // Paticia Battle
    ClipSet patriciaBattle{};
    patriciaBattle.nameByKey = {
        { AnimKey::Battle_Idle,             L"PC24A_01000" },
        { AnimKey::Battle_RunStart,         L"PC24A_01040" },
        { AnimKey::Battle_RunLoop,          L"PC24A_01041" },
        { AnimKey::Battle_RunEnd,           L"PC24A_01042" },
        { AnimKey::Battle_Celemony_1A,      L"PC24A_01050" },
        { AnimKey::Battle_Celemony_1B,      L"PC24A_01051" },
        { AnimKey::Battle_Celemony_2A,      L"PC24A_01060" },
        { AnimKey::Battle_Celemony_2B,      L"PC24A_01061" },
        { AnimKey::Battle_Attack_FinishedA, L"PC24A_01080" },
        { AnimKey::Battle_Attack_FinishedB, L"PC24A_01081" },
        { AnimKey::Battle_Attack_FinishedC, L"PC24A_01082" },
        { AnimKey::Battle_Skill_1A,         L"PC24A_01110" },
        { AnimKey::Battle_Skill_1B,         L"PC24A_01111" },
        { AnimKey::Battle_Skill_1C,         L"PC24A_01111" },
        { AnimKey::Battle_AttackA,          L"PC24A_01300" },
        { AnimKey::Battle_AttackB,          L"PC24A_01301" },
        { AnimKey::Battle_AttackC,          L"PC24A_01302" },
        { AnimKey::Battle_StartA,           L"PC24A_03030" },
        { AnimKey::Battle_StartB,           L"PC24A_03031" },
        { AnimKey::Battle_StartC,           L"PC24A_03032" },
    };
    Register(CharacterID::Patricia, AnimContext::Battle, patriciaBattle);
}

const ClipSet* AnimDataSystem::GetClipSet(CharacterID character, AnimContext context) const
{
    auto it = catalog.find(Key{ character, context });
    return (it == catalog.end()) ? nullptr : &it->second;
}

const wstring& AnimDataSystem::GetClipName(CharacterID character, AnimContext context, AnimKey key) const
{
    static const wstring empty = L"";
    if (const ClipSet* set = GetClipSet(character, context))
        return set->Require(key);
    return empty;
}