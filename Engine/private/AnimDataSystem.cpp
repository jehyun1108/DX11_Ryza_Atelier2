#include "Enginepch.h"

static inline ClipTuning NormalizedTuning(ClipTuning tuning)
{
    tuning.startNormalized = Utility::Saturate(tuning.startNormalized);
    tuning.endNormalized = Utility::Saturate(tuning.endNormalized);
    if (tuning.endNormalized < tuning.startNormalized)
        swap(tuning.endNormalized, tuning.startNormalized);
    if (fabsf(tuning.endNormalized - tuning.startNormalized) < 1e-5f)
        tuning.endNormalized = min(1.f, tuning.startNormalized + 1e-4f);
    return tuning;
}

void AnimDataSystem::OnBoot()
{
    RegisterDefaultClips();
}

void AnimDataSystem::RegisterDefaultClips()
{
	// Ryza - Field
	ClipSet ryzaField{};
    ryzaField.nameByKey = {
        { AnimKey::Idle,            L"PC20A_00000" },
        { AnimKey::WalkStart,       L"PC20A_00020" },
        { AnimKey::WalkLoop,        L"PC20A_00021" },
        { AnimKey::WalkEnd,         L"PC20A_00022" },
        { AnimKey::JumpStart,       L"PC20A_00070" },
        { AnimKey::JumpLoop,        L"PC20A_00071" },
        { AnimKey::JumpEnd,         L"PC20A_00072" },
        { AnimKey::RunStart,        L"PC20A_00130" },
        { AnimKey::RunLoop,         L"PC20A_00131" },
        { AnimKey::RunEnd,          L"PC20A_00132" },
        { AnimKey::FieldSwing,      L"PC20A_00200" },
        { AnimKey::Dressing_Change, L"PC20A_90090" },

        { AnimKey::WakeUp_A,        L"PC20A_90001" },
        { AnimKey::WakeUp_B,        L"PC20A_90010" },
        { AnimKey::WakeUp_C,        L"PC20A_90011" },
        { AnimKey::WakeUp_D,        L"PC20A_90020" },
        { AnimKey::WakeUp_E,        L"PC20A_90021" },
        { AnimKey::WakeUp_F,        L"PC20A_90022" },
        { AnimKey::WakeUp_G,        L"PC20A_90040" },
        { AnimKey::WakeUp_H,        L"PC20A_90041" },
        { AnimKey::WakeUp_I,        L"PC20A_90042" },
    };
    Register(CharacterID::Ryza, AnimContext::Field, ryzaField);
    // Ryza - Battle
    ClipSet ryzaBattle{};
    ryzaBattle.nameByKey = {
        { AnimKey::Battle_Idle,             L"PC20A_01000" },
        { AnimKey::Battle_RunStart,         L"PC20A_01040" },
        { AnimKey::Battle_RunLoop,          L"PC20A_01041" },
        { AnimKey::Battle_RunEnd,           L"PC20A_01042" },
        { AnimKey::Battle_Ceremony_1A,      L"PC20A_01050" },
        { AnimKey::Battle_Ceremony_1B,      L"PC20A_01051" },
        { AnimKey::Battle_Ceremony_2A,      L"PC20A_01060" },
        { AnimKey::Battle_Ceremony_2B,      L"PC20A_01061" },
        { AnimKey::Battle_Attack_FinishedA, L"PC20A_01080" },
        { AnimKey::Battle_Attack_FinishedB, L"PC20A_01081" },
        { AnimKey::Battle_Attack_FinishedC, L"PC20A_01082" },
        { AnimKey::Battle_Defend_Ready,     L"PC20A_01090" },
        { AnimKey::Battle_Defending,        L"PC20A_01091" },
        { AnimKey::Battle_Defend_Finished,  L"PC20A_01092" },
        { AnimKey::Battle_Defend_Success,   L"PC20A_01100" },
        { AnimKey::Battle_Skill_A1,         L"PC20A_01110" },
        { AnimKey::Battle_Skill_A2,         L"PC20A_01111" },
        { AnimKey::Battle_Skill_A3,         L"PC20A_01112" },
        { AnimKey::Battle_Skill_B1,         L"PC20A_01120" },
        { AnimKey::Battle_Skill_B2,         L"PC20A_01121" },
        { AnimKey::Battle_Skill_B3,         L"PC20A_01122" },
        { AnimKey::Battle_Skill_C1,         L"PC20A_01130" },
        { AnimKey::Battle_Skill_C2,         L"PC20A_01131" },
        { AnimKey::Battle_Skill_C3,         L"PC20A_01132" },
        { AnimKey::Battle_Skill_D1,         L"PC20A_01303" },
        { AnimKey::Battle_Skill_D2,         L"PC20A_01304" },
        { AnimKey::Battle_Skill_D3,         L"PC20A_01310" },
        { AnimKey::Battle_AttackA,          L"PC20A_01300" },
        { AnimKey::Battle_AttackB,          L"PC20A_01301" },
        { AnimKey::Battle_AttackC,          L"PC20A_01302" },
        { AnimKey::Battle_StartA,           L"PC20A_30130" },
        { AnimKey::Battle_StartB,           L"PC20A_30131" },
        { AnimKey::Battle_StartC,           L"PC20A_30132" },
        { AnimKey::Battle_Stunned,          L"PC20A_01011" },
        { AnimKey::Battle_Stun_End,         L"PC20A_01012" },
        { AnimKey::Battle_Hit,              L"PC20A_01020" },
        
    };
    Register(CharacterID::Ryza, AnimContext::Battle, ryzaBattle);
    // Kluadia Field
    ClipSet klaudiaField{};
    klaudiaField.nameByKey = {
        { AnimKey::Dressing_Idle,   L"PC21A_02720" },
        { AnimKey::Dressing_Change, L"PC21A_02710" },
    };
    Register(CharacterID::Klaudia, AnimContext::Field, klaudiaField);
    // Kluadia Battle
    ClipSet klaudiaBattle{};
    klaudiaBattle.nameByKey = {
        { AnimKey::Battle_Idle,               L"PC21A_01000" },
        { AnimKey::Battle_RunStart,           L"PC21A_01040" },
        { AnimKey::Battle_RunLoop,            L"PC21A_01041" },
        { AnimKey::Battle_RunEnd,             L"PC21A_01042" },
        { AnimKey::Battle_Ceremony_1A,        L"PC21A_01050" },
        { AnimKey::Battle_Ceremony_1B,        L"PC21A_01051" },
        { AnimKey::Battle_Ceremony_2A,        L"PC21A_01060" },
        { AnimKey::Battle_Ceremony_2B,        L"PC21A_01061" },
        { AnimKey::Battle_Attack_FinishedA,   L"PC21A_01080" },
        { AnimKey::Battle_Attack_FinishedB,   L"PC21A_01081" },
        { AnimKey::Battle_Attack_FinishedC,   L"PC21A_01082" },
        { AnimKey::Battle_Defend_Ready,       L"PC21A_01090" },
        { AnimKey::Battle_Defending,          L"PC21A_01091" },
        { AnimKey::Battle_Defend_Finished,    L"PC21A_01092" },
        { AnimKey::Battle_Defend_Success,     L"PC21A_01100" },
        { AnimKey::Battle_Skill_A1,           L"PC21A_01110" },
        { AnimKey::Battle_Skill_A2,           L"PC21A_01111" },
        { AnimKey::Battle_Skill_A3,           L"PC21A_01112" },
        { AnimKey::Battle_Skill_B1,           L"PC21A_01120" },
        { AnimKey::Battle_Skill_B2,           L"PC21A_01121" },
        { AnimKey::Battle_Skill_B3,           L"PC21A_01122" },
        { AnimKey::Battle_Skill_C1,           L"PC21A_01130" },
        { AnimKey::Battle_Skill_C2,           L"PC21A_01131" },
        { AnimKey::Battle_Skill_C3,           L"PC21A_01132" },
        { AnimKey::Battle_Skill_D1,           L"PC21A_01303" },
        { AnimKey::Battle_Skill_D2,           L"PC21A_01304" },
        { AnimKey::Battle_Skill_D3,           L"PC21A_01310" },
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
        { AnimKey::Battle_Stunned,            L"PC21A_01011" },
        { AnimKey::Battle_Stun_End,           L"PC21A_01012" },
        { AnimKey::Battle_Hit,                L"PC21A_01020" },
    };
    Register(CharacterID::Klaudia, AnimContext::Battle, klaudiaBattle);
    // Patricia Field
    ClipSet patriciaField{};
    patriciaField.nameByKey = {
        { AnimKey::Dressing_Idle,   L"PC24A_02680" },
        { AnimKey::Dressing_Change, L"PC24A_02710" },
    };
    Register(CharacterID::Patricia, AnimContext::Field, patriciaField);
    // Paticia Battle
    ClipSet patriciaBattle{};
    patriciaBattle.nameByKey = {
        { AnimKey::Battle_Idle,               L"PC24A_01000" },
        { AnimKey::Battle_RunStart,           L"PC24A_01040" },
        { AnimKey::Battle_RunLoop,            L"PC24A_01041" },
        { AnimKey::Battle_RunEnd,             L"PC24A_01042" },
        { AnimKey::Battle_Ceremony_1A,        L"PC24A_01050" },
        { AnimKey::Battle_Ceremony_1B,        L"PC24A_01051" },
        { AnimKey::Battle_Ceremony_2A,        L"PC24A_01060" },
        { AnimKey::Battle_Ceremony_2B,        L"PC24A_01061" },
        { AnimKey::Battle_Attack_FinishedA,   L"PC24A_01080" },
        { AnimKey::Battle_Attack_FinishedB,   L"PC24A_01081" },
        { AnimKey::Battle_Attack_FinishedC,   L"PC24A_01082" },
        { AnimKey::Battle_Defend_Ready,       L"PC24A_01090" },
        { AnimKey::Battle_Defending,          L"PC24A_01091" },
        { AnimKey::Battle_Defend_Finished,    L"PC24A_01092" },
        { AnimKey::Battle_Defend_Success,     L"PC24A_01100" },
        { AnimKey::Battle_Skill_A1,           L"PC24A_01110" },
        { AnimKey::Battle_Skill_A2,           L"PC24A_01111" },
        { AnimKey::Battle_Skill_A3,           L"PC24A_01112" },
        { AnimKey::Battle_Skill_B1,           L"PC24A_01120" },
        { AnimKey::Battle_Skill_B2,           L"PC24A_01121" },
        { AnimKey::Battle_Skill_B3,           L"PC24A_01122" },
        { AnimKey::Battle_Skill_C1,           L"PC24A_01130" },
        { AnimKey::Battle_Skill_C2,           L"PC24A_01131" },
        { AnimKey::Battle_Skill_C3,           L"PC24A_01132" },
        { AnimKey::Battle_Skill_D1,           L"PC24A_01303" },
        { AnimKey::Battle_Skill_D2,           L"PC24A_01304" },
        { AnimKey::Battle_Skill_D3,           L"PC24A_01310" },
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
        { AnimKey::Battle_Stunned,            L"PC24A_01011" },
        { AnimKey::Battle_Stun_End,           L"PC24A_01012" },
        { AnimKey::Battle_Hit,                L"PC24A_01020" },
    };
    Register(CharacterID::Patricia, AnimContext::Battle, patriciaBattle);
    // Angel Battle
    ClipSet angelBattle{};
    angelBattle.nameByKey = {
        { AnimKey::Battle_RunStart,    L"MOB02A_00011" },
        { AnimKey::Battle_RunLoop,     L"MOB02A_00011" },
        { AnimKey::Battle_RunEnd,      L"MOB02A_00011" },
        { AnimKey::Battle_Idle,        L"MOB02A_01000" },
        { AnimKey::Battle_AttackA,     L"MOB02A_01030" },
        { AnimKey::Battle_AttackB,     L"MOB02A_01030" },
        { AnimKey::Battle_AttackC,     L"MOB02A_01030" },
        { AnimKey::Battle_Skill_A1,    L"MOB02A_01040" },
        { AnimKey::Battle_Skill_A2,    L"MOB02A_01041" },
        { AnimKey::Battle_Skill_A3,    L"MOB02A_01042" },
        { AnimKey::Battle_Hit,         L"MOB02A_01010" },
        { AnimKey::Battle_Stunned,     L"MOB02A_01021" },
    };
    Register(CharacterID::Angel, AnimContext::Battle, angelBattle);

    SetDefaultTunings();
}

void AnimDataSystem::SetDefaultTunings()
{
// --------------------------------------------------- Patricia --------------------------------------------------------------------------------------
    SetClipTuning(CharacterID::Patricia, AnimContext::Battle, AnimKey::Battle_AttackA, ClipTuning{ .startNormalized = 0.1f, .endNormalized = 0.5f, .playbackSpeed = 1.3f });
    SetClipTuning(CharacterID::Patricia, AnimContext::Battle, AnimKey::Battle_AttackB, ClipTuning{ .startNormalized = 0.05f, .endNormalized = 0.5f, .playbackSpeed = 1.3f });
    SetClipTuning(CharacterID::Patricia, AnimContext::Battle, AnimKey::Battle_AttackC, ClipTuning{ .startNormalized = 0.05f, .endNormalized = 0.8f, .playbackSpeed = 1.3f });
    SetClipTuning(CharacterID::Patricia, AnimContext::Battle, AnimKey::Battle_Attack_FinishedA, ClipTuning{ .startNormalized = 0.2f, .endNormalized = 0.8f , .playbackSpeed = 1.2f });
    SetClipTuning(CharacterID::Patricia, AnimContext::Battle, AnimKey::Battle_Attack_FinishedB, ClipTuning{ .startNormalized = 0.2f, .endNormalized = 0.8f, .playbackSpeed = 1.2f });
    SetClipTuning(CharacterID::Patricia, AnimContext::Battle, AnimKey::Battle_Defend_Ready, ClipTuning{ .startNormalized = 0.2f, .endNormalized = 0.8f, .playbackSpeed = 1.2f });
    SetClipTuning(CharacterID::Patricia, AnimContext::Battle, AnimKey::Battle_Defending, ClipTuning{ .startNormalized = 0.2f, .endNormalized = 0.8f, .playbackSpeed = 1.2f });
    SetClipTuning(CharacterID::Patricia, AnimContext::Battle, AnimKey::Battle_Defend_Finished, ClipTuning{ .startNormalized = 0.2f, .endNormalized = 0.8f, .playbackSpeed = 1.2f });
    
    SetClipTuning(CharacterID::Patricia, AnimContext::Field, AnimKey::Dressing_Change, ClipTuning{ .playbackSpeed = 1.5f });

    //SetClipTuning(CharacterID::Patricia, AnimContext::Battle, AnimKey::Battle_Skill_A1, ClipTuning{ .playbackSpeed = 1.2f });
    //SetClipTuning(CharacterID::Patricia, AnimContext::Battle, AnimKey::Battle_Skill_A2, ClipTuning{ .playbackSpeed = 1.2f });
    //SetClipTuning(CharacterID::Patricia, AnimContext::Battle, AnimKey::Battle_Skill_A3, ClipTuning{ .playbackSpeed = 1.f });
    //SetClipTuning(CharacterID::Patricia, AnimContext::Battle, AnimKey::Battle_Skill_B1, ClipTuning{ .playbackSpeed = 1.2f });
    //SetClipTuning(CharacterID::Patricia, AnimContext::Battle, AnimKey::Battle_Skill_B2, ClipTuning{ .playbackSpeed = 1.2f });
    //SetClipTuning(CharacterID::Patricia, AnimContext::Battle, AnimKey::Battle_Skill_B3, ClipTuning{ .playbackSpeed = 1.f });
    //SetClipTuning(CharacterID::Patricia, AnimContext::Battle, AnimKey::Battle_Skill_C1, ClipTuning{ .playbackSpeed = 1.2f });
    //SetClipTuning(CharacterID::Patricia, AnimContext::Battle, AnimKey::Battle_Skill_C2, ClipTuning{ .playbackSpeed = 1.2f });
    //SetClipTuning(CharacterID::Patricia, AnimContext::Battle, AnimKey::Battle_Skill_C3, ClipTuning{ .playbackSpeed = 1.f });
    //SetClipTuning(CharacterID::Patricia, AnimContext::Battle, AnimKey::Battle_Skill_D1, ClipTuning{ .playbackSpeed = 1.2f });
    //SetClipTuning(CharacterID::Patricia, AnimContext::Battle, AnimKey::Battle_Skill_D2, ClipTuning{ .playbackSpeed = 1.2f });
    //SetClipTuning(CharacterID::Patricia, AnimContext::Battle, AnimKey::Battle_Skill_D3, ClipTuning{ .playbackSpeed = 1.f });
// ---------------------------------------------------- Ryza ----------------------------------------------------------------------------------------------
    SetClipTuning(CharacterID::Ryza, AnimContext::Battle, AnimKey::Battle_AttackA, ClipTuning{ .startNormalized = 0.05f, .endNormalized = 0.45f,
        .playbackSpeed = 1.f});
    SetClipTuning(CharacterID::Ryza, AnimContext::Battle, AnimKey::Battle_AttackB, ClipTuning{ .startNormalized = 0.1f, .endNormalized = 0.4f, 
        .playbackSpeed = 1.2f});
    SetClipTuning(CharacterID::Ryza, AnimContext::Battle, AnimKey::Battle_AttackC, ClipTuning{ .startNormalized = 0.1f, .endNormalized = 0.8f,
        .playbackSpeed = 1.3f});
    SetClipTuning(CharacterID::Ryza, AnimContext::Battle, AnimKey::Battle_Attack_FinishedA, ClipTuning{ .startNormalized = 0.2f, .endNormalized = 0.8f, .playbackSpeed = 1.2f });
    SetClipTuning(CharacterID::Ryza, AnimContext::Battle, AnimKey::Battle_Attack_FinishedB, ClipTuning{ .startNormalized = 0.2f, .endNormalized = 0.8f, .playbackSpeed = 1.2f });
    SetClipTuning(CharacterID::Ryza, AnimContext::Battle, AnimKey::Battle_Defend_Ready, ClipTuning{ .startNormalized = 0.2f, .endNormalized = 0.8f,
        .playbackSpeed = 1.2f });
    SetClipTuning(CharacterID::Ryza, AnimContext::Battle, AnimKey::Battle_Defending, ClipTuning{ .startNormalized = 0.2f, .endNormalized = 0.8f, 
        .playbackSpeed = 1.2f });
    SetClipTuning(CharacterID::Ryza, AnimContext::Battle, AnimKey::Battle_Defend_Finished, ClipTuning{ .startNormalized = 0.2f, .endNormalized = 0.8f, .playbackSpeed = 1.2f });

    SetClipTuning(CharacterID::Ryza, AnimContext::Battle, AnimKey::Battle_Skill_A1, ClipTuning{ .playbackSpeed = 1.2f });
    SetClipTuning(CharacterID::Ryza, AnimContext::Battle, AnimKey::Battle_Skill_A2, ClipTuning{ .playbackSpeed = 1.2f });
    SetClipTuning(CharacterID::Ryza, AnimContext::Battle, AnimKey::Battle_Skill_A3, ClipTuning{ .playbackSpeed = 1.f });
    SetClipTuning(CharacterID::Ryza, AnimContext::Battle, AnimKey::Battle_Skill_B1, ClipTuning{ .playbackSpeed = 1.2f });
    SetClipTuning(CharacterID::Ryza, AnimContext::Battle, AnimKey::Battle_Skill_B2, ClipTuning{ .playbackSpeed = 1.2f });
    SetClipTuning(CharacterID::Ryza, AnimContext::Battle, AnimKey::Battle_Skill_B3, ClipTuning{ .playbackSpeed = 1.f });
    SetClipTuning(CharacterID::Ryza, AnimContext::Battle, AnimKey::Battle_Skill_C1, ClipTuning{ .playbackSpeed = 1.2f });
    SetClipTuning(CharacterID::Ryza, AnimContext::Battle, AnimKey::Battle_Skill_C2, ClipTuning{ .playbackSpeed = 1.2f });
    SetClipTuning(CharacterID::Ryza, AnimContext::Battle, AnimKey::Battle_Skill_C3, ClipTuning{ .playbackSpeed = 1.f });
    SetClipTuning(CharacterID::Ryza, AnimContext::Battle, AnimKey::Battle_Skill_D1, ClipTuning{ .playbackSpeed = 1.2f });
    SetClipTuning(CharacterID::Ryza, AnimContext::Battle, AnimKey::Battle_Skill_D2, ClipTuning{ .playbackSpeed = 1.2f });
    SetClipTuning(CharacterID::Ryza, AnimContext::Battle, AnimKey::Battle_Skill_D3, ClipTuning{ .playbackSpeed = 1.f });

    // ==================================================================================================================================================

// ----------------------------------------------------- Klaudia --------------------------------------------------------------------------------------------
    SetClipTuning(CharacterID::Klaudia, AnimContext::Battle, AnimKey::Battle_AttackA, ClipTuning{ .startNormalized = 0.05f, .endNormalized = 0.6f,
        .playbackSpeed = 1.2f});
    SetClipTuning(CharacterID::Klaudia, AnimContext::Battle, AnimKey::Battle_AttackB, ClipTuning{ .startNormalized = 0.05f, .endNormalized = 0.6f,
        .playbackSpeed = 1.2f });
    SetClipTuning(CharacterID::Klaudia, AnimContext::Battle, AnimKey::Battle_AttackC, ClipTuning{ .startNormalized = 0.05f, .endNormalized = 0.8f,
        .playbackSpeed = 1.2f });
    SetClipTuning(CharacterID::Klaudia, AnimContext::Battle, AnimKey::Battle_Attack_FinishedA, ClipTuning{ .startNormalized = 0.2f, .endNormalized = 0.8f, .playbackSpeed = 1.2f });
    SetClipTuning(CharacterID::Klaudia, AnimContext::Battle,  AnimKey::Battle_Attack_FinishedB, ClipTuning{ .startNormalized = 0.2f, .endNormalized = 0.8f, .playbackSpeed = 1.2f });
    SetClipTuning(CharacterID::Klaudia, AnimContext::Battle, AnimKey::Battle_Defend_Ready, ClipTuning{ .startNormalized = 0.2f, .endNormalized = 0.8f,
        .playbackSpeed = 1.2f });
    SetClipTuning(CharacterID::Klaudia, AnimContext::Battle, AnimKey::Battle_Defending, ClipTuning{ .startNormalized = 0.2f, .endNormalized = 0.8f,
        .playbackSpeed = 1.2f });
    SetClipTuning(CharacterID::Klaudia, AnimContext::Battle, AnimKey::Battle_Defend_Finished, ClipTuning{ .startNormalized = 0.2f, .endNormalized = 0.8f, .playbackSpeed = 1.2f });

    //SetClipTuning(CharacterID::Klaudia, AnimContext::Battle, AnimKey::Battle_Skill_A1, ClipTuning{ .playbackSpeed = 1.2f });
    //SetClipTuning(CharacterID::Klaudia, AnimContext::Battle, AnimKey::Battle_Skill_A2, ClipTuning{ .playbackSpeed = 1.2f });
    //SetClipTuning(CharacterID::Klaudia, AnimContext::Battle, AnimKey::Battle_Skill_A3, ClipTuning{ .playbackSpeed = 1.f });
    //SetClipTuning(CharacterID::Klaudia, AnimContext::Battle, AnimKey::Battle_Skill_B1, ClipTuning{ .playbackSpeed = 1.2f });
    //SetClipTuning(CharacterID::Klaudia, AnimContext::Battle, AnimKey::Battle_Skill_B2, ClipTuning{ .playbackSpeed = 1.2f });
    //SetClipTuning(CharacterID::Klaudia, AnimContext::Battle, AnimKey::Battle_Skill_B3, ClipTuning{ .playbackSpeed = 1.f });
    //SetClipTuning(CharacterID::Klaudia, AnimContext::Battle, AnimKey::Battle_Skill_C1, ClipTuning{ .playbackSpeed = 1.2f });
    //SetClipTuning(CharacterID::Klaudia, AnimContext::Battle, AnimKey::Battle_Skill_C2, ClipTuning{ .playbackSpeed = 1.2f });
    //SetClipTuning(CharacterID::Klaudia, AnimContext::Battle, AnimKey::Battle_Skill_C3, ClipTuning{ .playbackSpeed = 1.f });
    //SetClipTuning(CharacterID::Klaudia, AnimContext::Battle, AnimKey::Battle_Skill_D1, ClipTuning{ .playbackSpeed = 1.2f });
    //SetClipTuning(CharacterID::Klaudia, AnimContext::Battle, AnimKey::Battle_Skill_D2, ClipTuning{ .playbackSpeed = 1.2f });
    //SetClipTuning(CharacterID::Klaudia, AnimContext::Battle, AnimKey::Battle_Skill_D3, ClipTuning{ .playbackSpeed = 1.f });
    // ----------------------------------------------------- Angel --------------------------------------------------------------------------------------------

}

const ClipSet* AnimDataSystem::GetClipSet(CharacterID character, AnimContext context) const
{
    auto it = catalog.find(Key{ character, context });
    return (it == catalog.end()) ? nullptr : &it->second;
}

const wstring& AnimDataSystem::GetClipName(CharacterID character, AnimContext context, AnimKey key) const
{
    return GetClipSet(character, context)->Require(key);
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