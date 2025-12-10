#pragma once

NS_BEGIN(Engine)

enum class FadeMode
{
    White, Black
};
enum class CamOwnerType
{
    Default, 
    Character
};
enum class CamRole
{
    BasicAttack,
    SkillSlot, 
    Ultimate,
    BattleIntro,
    Reward,
    CutScene
};
struct CamMeta
{
    CamOwnerType   ownerType   = CamOwnerType::Default;
    CharacterID    characterId = CharacterID::Unknown;
    CamRole        role        = CamRole::BasicAttack;
    SpecialAnimTag specialTag  = SpecialAnimTag::None;
};
enum class BattleCamKey
{
	Default_Follow, Intro_Action
};
struct CamKey
{
    float   t      = 0.f;
    _float3 pos    = {};
    _float3 look   = {};
    float   fovDeg = 60.f;
};

struct CamShakeEvent
{
    float t{};
    float width{};
    float amp{};
    float yScale{};
};
struct SeqCamPreset
{
    float                 duration = 0.f;
    vector<CamKey>        keys;
    vector<CamShakeEvent> shakes;
    vector<ShotClip>      clips;
    CamMeta               meta;
};
struct CamFadeProfile
{
    bool     useFade = false;
    FadeMode mode    = FadeMode::Black;
    float    inDur   = 0.2f;
    float    outDur  = 0.2f;
};

NS_END