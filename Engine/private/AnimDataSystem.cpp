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
        { AnimKey::Battle_Defend_Ready,     L"PC20A_01090" },
        { AnimKey::Battle_Defending,        L"PC20A_01091" },
        { AnimKey::Battle_Defend_Finished,  L"PC20A_01092" },
        { AnimKey::Battle_Skill_A1,         L"PC20A_01110" },
        { AnimKey::Battle_Skill_A1,         L"PC20A_01111" },
        { AnimKey::Battle_Skill_A1,         L"PC20A_01112" },
        { AnimKey::Battle_Skill_B1,         L"PC20A_01120" },
        { AnimKey::Battle_Skill_B2,         L"PC20A_01121" },
        { AnimKey::Battle_Skill_B3,         L"PC20A_01122" },
        { AnimKey::Battle_Skill_C1,         L"PC20A_01130" },
        { AnimKey::Battle_Skill_C2,         L"PC20A_01131" },
        { AnimKey::Battle_Skill_C3,         L"PC20A_01132" },
        { AnimKey::Battle_Skill_D1,         L"PC20A_01140" },
        { AnimKey::Battle_Skill_D2,         L"PC20A_01141" },
        { AnimKey::Battle_Skill_D3,         L"PC20A_01142" },
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
    Register(CharacterID::Klaudia, AnimContext::Field, klaudiaField);
    // Kluadia Battle
    ClipSet klaudiaBattle{};
    klaudiaBattle.nameByKey = {
        { AnimKey::Battle_Idle,               L"PC21A_01000" },
        { AnimKey::Battle_RunStart,           L"PC21A_01040" },
        { AnimKey::Battle_RunLoop,            L"PC21A_01041" },
        { AnimKey::Battle_RunEnd,             L"PC21A_01042" },
        { AnimKey::Battle_Celemony_1A,        L"PC21A_01050" },
        { AnimKey::Battle_Celemony_1B,        L"PC21A_01051" },
        { AnimKey::Battle_Celemony_2A,        L"PC21A_01060" },
        { AnimKey::Battle_Celemony_2B,        L"PC21A_01061" },
        { AnimKey::Battle_Attack_FinishedA,   L"PC21A_01080" },
        { AnimKey::Battle_Attack_FinishedB,   L"PC21A_01081" },
        { AnimKey::Battle_Attack_FinishedC,   L"PC21A_01082" },
        { AnimKey::Battle_Defend_Ready,       L"PC21A_01090" },
        { AnimKey::Battle_Defending,          L"PC21A_01091" },
        { AnimKey::Battle_Defend_Finished,    L"PC21A_01092" },
        { AnimKey::Battle_Skill_A1,           L"PC21A_01110" },
        { AnimKey::Battle_Skill_A1,           L"PC21A_01111" },
        { AnimKey::Battle_Skill_A1,           L"PC21A_01112" },
        { AnimKey::Battle_Skill_B1,           L"PC21A_01120" },
        { AnimKey::Battle_Skill_B2,           L"PC21A_01121" },
        { AnimKey::Battle_Skill_B3,           L"PC21A_01122" },
        { AnimKey::Battle_Skill_C1,           L"PC21A_01130" },
        { AnimKey::Battle_Skill_C2,           L"PC21A_01131" },
        { AnimKey::Battle_Skill_C3,           L"PC21A_01132" },
        { AnimKey::Battle_Skill_D1,           L"PC21A_01140" },
        { AnimKey::Battle_Skill_D2,           L"PC21A_01141" },
        { AnimKey::Battle_Skill_D3,           L"PC21A_01142" },
        { AnimKey::Battle_Ultimate_Klaudia1,  L"PC21A_01200" },
        { AnimKey::Battle_Ultimate_Klaudia2,  L"PC21A_01201" },
        { AnimKey::Battle_Ultimate_Klaudia3,  L"PC21A_01202" },
        { AnimKey::Battle_Ultimate_Klaudia4,  L"PC21A_01203" },
        { AnimKey::Battle_Ultimate_Klaudia5,  L"PC21A_01204" },
        { AnimKey::Battle_Ultimate_Klaudia6,  L"PC21A_01205" },
        { AnimKey::Battle_Ultimate_Klaudia7,  L"PC21A_01206" },
        { AnimKey::Battle_Ultimate_Klaudia8,  L"PC21A_01207" },
        { AnimKey::Battle_Ultimate_Klaudia9,  L"PC21A_01208" },
        { AnimKey::Battle_AttackA,            L"PC21A_01300" },
        { AnimKey::Battle_AttackB,            L"PC21A_01301" },
        { AnimKey::Battle_AttackC,            L"PC21A_01302" },
        { AnimKey::Battle_StartA,             L"PC21A_03030" },
        { AnimKey::Battle_StartB,             L"PC21A_03031" },
        { AnimKey::Battle_StartC,             L"PC21A_03032" },
    };
    Register(CharacterID::Klaudia, AnimContext::Battle, klaudiaBattle);
    // Patricia Field
    ClipSet patriciaField{};
    patriciaField.nameByKey = {

    };
    Register(CharacterID::Patricia, AnimContext::Field, patriciaField);
    // Paticia Battle
    ClipSet patriciaBattle{};
    patriciaBattle.nameByKey = {
        { AnimKey::Battle_Idle,               L"PC24A_01000" },
        { AnimKey::Battle_RunStart,           L"PC24A_01040" },
        { AnimKey::Battle_RunLoop,            L"PC24A_01041" },
        { AnimKey::Battle_RunEnd,             L"PC24A_01042" },
        { AnimKey::Battle_Celemony_1A,        L"PC24A_01050" },
        { AnimKey::Battle_Celemony_1B,        L"PC24A_01051" },
        { AnimKey::Battle_Celemony_2A,        L"PC24A_01060" },
        { AnimKey::Battle_Celemony_2B,        L"PC24A_01061" },
        { AnimKey::Battle_Attack_FinishedA,   L"PC24A_01080" },
        { AnimKey::Battle_Attack_FinishedB,   L"PC24A_01081" },
        { AnimKey::Battle_Attack_FinishedC,   L"PC24A_01082" },
        { AnimKey::Battle_Defend_Ready,       L"PC24A_01090" },
        { AnimKey::Battle_Defending,          L"PC24A_01091" },
        { AnimKey::Battle_Defend_Finished,    L"PC24A_01092" },
        { AnimKey::Battle_Skill_A1,           L"PC24A_01110" },
        { AnimKey::Battle_Skill_A1,           L"PC24A_01111" },
        { AnimKey::Battle_Skill_A1,           L"PC24A_01112" },
        { AnimKey::Battle_Skill_B1,           L"PC24A_01120" },
        { AnimKey::Battle_Skill_B2,           L"PC24A_01121" },
        { AnimKey::Battle_Skill_B3,           L"PC24A_01122" },
        { AnimKey::Battle_Skill_C1,           L"PC24A_01130" },
        { AnimKey::Battle_Skill_C2,           L"PC24A_01131" },
        { AnimKey::Battle_Skill_C3,           L"PC24A_01132" },
        { AnimKey::Battle_Skill_D1,           L"PC24A_01140" },
        { AnimKey::Battle_Skill_D2,           L"PC24A_01141" },
        { AnimKey::Battle_Skill_D3,           L"PC24A_01142" },
        { AnimKey::Battle_Ultimate_Patricia1, L"PC24A_01200" },
        { AnimKey::Battle_Ultimate_Patricia2, L"PC24A_01201" },
        { AnimKey::Battle_Ultimate_Patricia3, L"PC24A_01202" },
        { AnimKey::Battle_Ultimate_Patricia4, L"PC24A_01203" },
        { AnimKey::Battle_Ultimate_Patricia5, L"PC24A_01204" },
        { AnimKey::Battle_Ultimate_Patricia6, L"PC24A_01205" },
        { AnimKey::Battle_Ultimate_Patricia7, L"PC24A_01206" },
        { AnimKey::Battle_AttackA,            L"PC24A_01300" },
        { AnimKey::Battle_AttackB,            L"PC24A_01301" },
        { AnimKey::Battle_AttackC,            L"PC24A_01302" },
        { AnimKey::Battle_StartA,             L"PC24A_03030" },
        { AnimKey::Battle_StartB,             L"PC24A_03031" },
        { AnimKey::Battle_StartC,             L"PC24A_03032" },
    };
    Register(CharacterID::Patricia, AnimContext::Battle, patriciaBattle);
    // Angel Battle
    ClipSet angelBattle{};
    angelBattle.nameByKey = {
        { AnimKey::Battle_Idle,      L"MOB02A_01000"},
        { AnimKey::Battle_AttackA,   L"MOB02A_01030"},
        { AnimKey::Battle_Skill_A1,  L"MOB02A_01040"},
        { AnimKey::Battle_Skill_A2,  L"MOB02A_01041" },
        { AnimKey::Battle_Skill_A3,  L"MOB02A_01042" },
    };
    Register(CharacterID::Angel, AnimContext::Battle, angelBattle);

    SetDefaultTunings();
}

void AnimDataSystem::SetDefaultTunings()
{
    SetClipTuning(CharacterID::Patricia, AnimContext::Battle, AnimKey::Battle_AttackA, ClipTuning{. startNormalized = 0.05f, .endNormalized = 0.4f });
    SetClipTuning(CharacterID::Patricia, AnimContext::Battle, AnimKey::Battle_AttackB, ClipTuning{ .startNormalized = 0.f, .endNormalized = 0.5f });
    SetClipTuning(CharacterID::Patricia, AnimContext::Battle, AnimKey::Battle_AttackC, ClipTuning{ .startNormalized = 0.f, .endNormalized = 1.f });
    
    SetClipTuning(CharacterID::Ryza, AnimContext::Battle, AnimKey::Battle_AttackA, ClipTuning{ .startNormalized = 0.1f, .endNormalized = 0.5f,
        .playbackSpeed = 1.2f});
    SetClipTuning(CharacterID::Ryza, AnimContext::Battle, AnimKey::Battle_AttackB, ClipTuning{ .startNormalized = 0.1f, .endNormalized = 0.5f, 
        .playbackSpeed = 0.8f});
    SetClipTuning(CharacterID::Ryza, AnimContext::Battle, AnimKey::Battle_AttackC, ClipTuning{ .startNormalized = 0.1f, .endNormalized = 1.f,
        .playbackSpeed = 1.2f});

    SetClipTuning(CharacterID::Klaudia, AnimContext::Battle, AnimKey::Battle_AttackA, ClipTuning{ .startNormalized = 0.05f, .endNormalized = 0.6f });
    SetClipTuning(CharacterID::Klaudia, AnimContext::Battle, AnimKey::Battle_AttackB, ClipTuning{ .startNormalized = 0.05f, .endNormalized = 0.6f });
    SetClipTuning(CharacterID::Klaudia, AnimContext::Battle, AnimKey::Battle_AttackC, ClipTuning{ .startNormalized = 0.05f, .endNormalized = 1.f });
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

void AnimDataSystem::SetClipTuning(CharacterID character, AnimContext context, AnimKey key, const ClipTuning& tuning)
{
    auto it = catalog.find(Key{ character, context });
    if (it == catalog.end()) return;
    it->second.tuningByKey[key] = NormalizedTuning(tuning);
}

ClipTuning AnimDataSystem::GetClipTuning(CharacterID character, AnimContext context, AnimKey key) const
{
    if (const ClipSet* set = GetClipSet(character, context))
        return set->ResolveTuning(key);
    return ClipTuning{};
}