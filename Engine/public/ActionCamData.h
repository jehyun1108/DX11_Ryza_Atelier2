#pragma once

NS_BEGIN(Engine)

using ActionId = BattleCommand;
struct ActionCamSpec
{
    ClipId          clipId       = 0;
    ActionCamAnchor anchor       = ActionCamAnchor::None;
    CamPriority     priority     = CamPriority::High;
    CamLayer        layer        = CamLayer::Action;
    bool            lockUntilEnd = true;
    float           fovOverride  = 0.f;
};
struct ActionDef
{
    ActionId      actionId{};
    AnimKey       animKey{};
    ActionCamSpec cam{};
};
struct ActionCamKey
{
    CharacterID    character;
    SpecialAnimTag tag;
    bool operator==(const ActionCamKey& other) const noexcept
    {
        return character == other.character && tag == other.tag;
    }
};
struct ActionCamKeyHasher
{
    size_t operator()(const ActionCamKey& key) const noexcept
    {
        size_t hash = 0;
        hash ^= (size_t)key.character + 0x9e3779b97f4a7c15ull + (hash << 6) + (hash >> 2);
        hash ^= (size_t)key.tag       + 0x9e3779b97f4a7c15ull + (hash << 6) + (hash >> 2);
        return hash;
    }
};
struct ActionCamEntry
{
    ActionCamSpec    spec;
    vector<ShotClip> clips;
};

NS_END