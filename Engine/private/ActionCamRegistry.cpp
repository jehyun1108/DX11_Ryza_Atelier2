#include "Enginepch.h"

void ActionCamRegistry::OnBoot()
{
	camReg       = &registry.Get<CamRegistry>();
	formationSys = &registry.Get<BattleFormationSystem>();
    dataSys      = &registry.Get<CharacterDataSystem>();

}

const ActionDef* ActionCamRegistry::FindCommand(ActionId id) const
{
    auto it = defs.find(id);
    if (it == defs.end())
        return nullptr;
    return &it->second;
}

TrackID ActionCamRegistry::PlayActionCam(ActionId action, EntityID attacker, EntityID victim)
{
    ActionCamSpec spec{};

    if (action == BattleCommand::AttackBasic)
    {
        CharacterID ch = dataSys->GetCharacterID(attacker);
        ClipId clip = camReg->FindBasicAttackCam(ch);

        if (clip != 0)
        {
            spec.clipId = clip;
            spec.anchor = ActionCamAnchor::Attacker;
            spec.priority = CamPriority::High;
            spec.layer = CamLayer::Action;
            spec.lockUntilEnd = true;   
            spec.fovOverride = 0.f;
        }
        else
        {
            const ActionDef* def = FindCommand(action);
            if (!def) return {};
            spec = def->cam;
        }
    }
    else
    {
        const ActionDef* def = FindCommand(action);
        if (!def) return {};
        spec = def->cam;
    }

    if (spec.clipId == 0)
        return {};

    TrackSpawnRequest req = BuildTrackRequest(spec, attacker, victim);
    const bool trackRecent = spec.lockUntilEnd;
    return camReg->Spawn(req, trackRecent);
}

void ActionCamRegistry::RegisterSkillCam(CharacterID character, SpecialAnimTag tag, const ActionCamSpec& spec, const vector<ShotClip>& clips)
{
    ActionCamKey key{};
    key.character = character;
    key.tag = tag;

    ActionCamEntry entry{};
    entry.spec = spec;
    entry.clips = clips;

    skillCams[key] = entry;

    if (!clips.empty())
        camReg->RegisterSeqClips(spec.clipId, clips);
}

const ActionCamEntry* ActionCamRegistry::FindSkill(CharacterID character, SpecialAnimTag tag) const
{
    ActionCamKey key{};
    key.character = character;
    key.tag = tag;

    auto it = skillCams.find(key);
    if (it == skillCams.end())
        return nullptr;
    return &it->second;
}

TrackID ActionCamRegistry::PlaySkillCam(CharacterID character, SpecialAnimTag tag, EntityID attacker, EntityID victim)
{
    const ActionCamEntry* entry = FindSkill(character, tag);
    if (!entry)
        return {};

    const ActionCamSpec& spec = entry->spec;
    if (spec.clipId == 0)
        return {};

    TrackSpawnRequest req = BuildTrackRequest(spec, attacker, victim);

    const bool trackRecent = spec.lockUntilEnd;
    TrackID id = camReg->Spawn(req, trackRecent);

    return id;
}

void ActionCamRegistry::StopTrack(TrackID id)
{
    if (!id.IsValid())
        return;
    camReg->StopActionTrack(id);
}

AnchorBinding ActionCamRegistry::BuildAnchor(const ActionCamSpec& spec, EntityID attacker, EntityID victim) const
{
    AnchorBinding anchor{};

    switch (spec.anchor)
    {
    case ActionCamAnchor::Attacker:
        anchor.space   = AnchorSpace::Target;
        anchor.binding = TargetBinding::CustomEntity;
        anchor.entity  = attacker;
        anchor.offset  = _float3{ 0.f, 0.f, 0.f }; 
        break;

    case ActionCamAnchor::Victim:
        anchor.space   = AnchorSpace::Target;
        anchor.binding = TargetBinding::CustomEntity;
        anchor.entity  = victim;
        anchor.offset  = _float3{ 0.f, 140.f, 220.f };
        break;

    case ActionCamAnchor::MidPoint:
    {
        anchor.space   = AnchorSpace::World;
        anchor.binding = TargetBinding::None;
        _float3 center = formationSys->GetCenter();
        anchor.offset  = { center.x + 0.f, center.y + 200.f, center.z - 300.f };
        break;
    }

    default:
        anchor.space   = AnchorSpace::Target;
        anchor.binding = TargetBinding::Leader;
        anchor.entity  = 0;
        anchor.offset  = _float3{ 0.f, 180.f, -250.f };
        break;
    }

    return anchor;
}

TrackSpawnRequest ActionCamRegistry::BuildTrackRequest(const ActionCamSpec& spec, EntityID attacker, EntityID victim) const
{
    TrackSpawnRequest req{};

    req.type = CamTrackType::Sequence;
    req.priority = spec.priority;
    req.layer = spec.layer;
    req.anchor = BuildAnchor(spec, attacker, victim);

    req.seqDesc.clipId = spec.clipId;
    req.seqDesc.lockUntilEnd = spec.lockUntilEnd;
    req.seqDesc.loop = false;
    req.seqDesc.timeScale = 1.f;

    return req;
}