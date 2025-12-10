#include "pch.h"
#include "EffectLoader.h"
#include "EffectSerializer.h"
#include "EffectSystem.h"
#include "ActionFxRegistry.h"

void EffectLoader::LoadEffect(SystemRegistry& registry)
{
    auto& assets     = registry.Get<AssetSystem>();
    auto& serializer = registry.Get<EffectSerializer>();
    auto& effectSys  = registry.Get<EffectSystem>();
    auto& fxReg      = registry.Get<ActionFxRegistry>();

    const wchar_t* effectPaths[] =
    {
        L"../bin/Resources/Particles/prefab/ryza_trail.effect",
        L"../bin/Resources/Particles/prefab/patricia_trail.effect",
        //L"../bin/Resources/Particles/prefab/ryza_hit.effect"
        L"../bin/Resources/Particles/prefab/fire_trail.effect",
        L"../bin/Resources/Particles/prefab/patricia_trail_glow.effect",
        L"../bin/Resources/Particles/prefab/patricia_trail_heart.effect",
        L"../bin/Resources/Particles/prefab/patricia_trail_star.effect",
    };

    for (auto path : effectPaths)
    {
        EffectArchetype effect;
        bool ok = serializer.Load(effect, path);
        assert(ok);

        assets.EnsureEffectTextures(effect);
        effectSys.RegisterArchetype(effect);
    }

    // ========================================================================================================
    
    LoadRyzaEffect(registry);
    LoadPatriciaEffect(registry);
    LoadKlaudiaEffect(registry);
}

void EffectLoader::LoadRyzaEffect(SystemRegistry& registry)
{
    auto& fxReg = registry.Get<ActionFxRegistry>();

    struct TrailDef
    {
        SpecialAnimTag              tag;
        initializer_list<AnimKey>   clips;
    };

    const TrailDef defs[] =
    {
        { SpecialAnimTag::BasicAttack,
        { AnimKey::Battle_AttackA, AnimKey::Battle_AttackB, AnimKey::Battle_AttackC } },

        { SpecialAnimTag::SkillA_1, { AnimKey::Battle_Skill_A1 } },
        { SpecialAnimTag::SkillA_2, { AnimKey::Battle_Skill_A2 } },
        { SpecialAnimTag::SkillA_3, { AnimKey::Battle_Skill_A3 } },

        { SpecialAnimTag::SkillB_1, { AnimKey::Battle_Skill_B1 } },
        { SpecialAnimTag::SkillB_2, { AnimKey::Battle_Skill_B2 } },
        { SpecialAnimTag::SkillB_3, { AnimKey::Battle_Skill_B3 } },

        { SpecialAnimTag::SkillC_1, { AnimKey::Battle_Skill_C1 } },
        { SpecialAnimTag::SkillC_2, { AnimKey::Battle_Skill_C2 } },
        { SpecialAnimTag::SkillC_3, { AnimKey::Battle_Skill_C3 } },

        { SpecialAnimTag::SkillD_1, { AnimKey::Battle_Skill_D1 } },
        { SpecialAnimTag::SkillD_2, { AnimKey::Battle_Skill_D2 } },
        { SpecialAnimTag::SkillD_3, { AnimKey::Battle_Skill_D3 } },
    };

    for (const TrailDef& def : defs)
    {
        ActionFxSet fx{};

        for (AnimKey clipKey : def.clips)
        {
            ActionTrailClipFx trail{};
            trail.clipKey   = clipKey;
            trail.effectKey = L"ryza_trail";
            trail.startNorm = 0.f;
            trail.endNorm   = 1.f;
            trail.attachToWeapon = true;
            fx.trails.push_back(trail);
        }

        fxReg.RegisterFx(CharacterID::Ryza, def.tag, fx);
    }
}

void EffectLoader::LoadPatriciaEffect(SystemRegistry& registry)
{
    auto& fxReg = registry.Get<ActionFxRegistry>();

    struct TrailDef
    {
        SpecialAnimTag              tag;
        initializer_list<AnimKey>   clips;
        const wchar_t* effectKey;
    };

    const TrailDef defs[] =
    {
        { SpecialAnimTag::BasicAttack,
        { AnimKey::Battle_AttackA, AnimKey::Battle_AttackB, AnimKey::Battle_AttackC }, L"patricia_trail_glow" },

        { SpecialAnimTag::SkillA_1, { AnimKey::Battle_Skill_A1 }, L"patricia_trail_glow" },
        { SpecialAnimTag::SkillA_2, { AnimKey::Battle_Skill_A2 }, L"patricia_trail_glow" },
        { SpecialAnimTag::SkillA_3, { AnimKey::Battle_Skill_A3 }, L"patricia_trail_glow" },

        { SpecialAnimTag::SkillB_1, { AnimKey::Battle_Skill_B1 }, L"fire_trail" },
        { SpecialAnimTag::SkillB_2, { AnimKey::Battle_Skill_B2 }, L"fire_trail" },
        { SpecialAnimTag::SkillB_3, { AnimKey::Battle_Skill_B3 }, L"fire_trail" },

        { SpecialAnimTag::SkillC_1, { AnimKey::Battle_Skill_C1 }, L"patricia_trail_heart" },
        { SpecialAnimTag::SkillC_2, { AnimKey::Battle_Skill_C2 }, L"patricia_trail_heart" },
        { SpecialAnimTag::SkillC_3, { AnimKey::Battle_Skill_C3 }, L"patricia_trail_heart" },

        { SpecialAnimTag::SkillD_1, { AnimKey::Battle_Skill_D1 }, L"patricia_trail_star" },
        { SpecialAnimTag::SkillD_2, { AnimKey::Battle_Skill_D2 }, L"patricia_trail_star" },
        { SpecialAnimTag::SkillD_3, { AnimKey::Battle_Skill_D3 }, L"patricia_trail_star" },
    };

    for (const TrailDef& def : defs)
    {
        ActionFxSet fx{};

        for (AnimKey clipKey : def.clips)
        {
            ActionTrailClipFx trail{};
            trail.clipKey = clipKey;
            trail.effectKey = def.effectKey; // 여기서 바로 사용
            trail.startNorm = 0.f;
            trail.endNorm = 1.f;
            trail.attachToWeapon = true;
            fx.trails.push_back(trail);
        }

        fxReg.RegisterFx(CharacterID::Patricia, def.tag, fx);
    }
}

void EffectLoader::LoadKlaudiaEffect(SystemRegistry& registry)
{
    auto& fxReg = registry.Get<ActionFxRegistry>();

}
