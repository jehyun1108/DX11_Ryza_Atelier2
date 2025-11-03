#include "Enginepch.h"

BattleTargetSystem::BattleTargetSystem(SystemRegistry& registry) : EntitySystem<Target>(registry)
{
    random_device rand{};
    rng.seed(rand());
}

void BattleTargetSystem::Init()
{
    auto& sessionSys = registry.Get<BattleSessionSystem>();
    const auto* state = sessionSys.TryGetState();
    if (!state) return;

    for (int i = 0; i < state->allies.memberCount; ++i)  if (state->allies.members[i]  != invalidEntity) Ensure(state->allies.members[i]);
    for (int i = 0; i < state->enemies.memberCount; ++i) if (state->enemies.members[i] != invalidEntity) Ensure(state->enemies.members[i]);

    Pair();
}

void BattleTargetSystem::OnUnitDowned(EntityID downed)
{
    if (downed == invalidEntity) return;
    auto& sessionSys  = registry.Get<BattleSessionSystem>();
    const auto* state = sessionSys.TryGetState();
    if (!state) return;

    auto retargetSide = [&](const BattleSide& side, BattleTeam team) 
        {
            vector<EntityID> pool = Opponents(team);
            for (int i = 0; i < side.memberCount; ++i)
            {
                EntityID ally = side.members[i];
                if (ally == invalidEntity) continue;
                Target* target = GetByOwner(ally);
                if (!target) continue;
                if (target->curTarget == downed)
                    target->curTarget = Pick(pool);
            }
        };
    retargetSide(state->allies,  BattleTeam::Ally);
    retargetSide(state->enemies, BattleTeam::Enemy);
}

EntityID BattleTargetSystem::Get(EntityID attacker) const
{
    const Target* target = const_cast<BattleTargetSystem*>(this)->GetByOwner(attacker);
    return target ? target->curTarget : invalidEntity;
}

void BattleTargetSystem::Ensure(EntityID owner)
{
    if (owner == invalidEntity) return;
    if (!GetByOwner(owner)) CreateComp(owner);
}

vector<EntityID> BattleTargetSystem::Opponents(BattleTeam team) const
{
    vector<EntityID> out;
    out.reserve(3);
    auto& sessionSys             = registry.Get<BattleSessionSystem>();
    const BattleParty* allies    = sessionSys.GetAllies();
    const BattleEnemies* enemies = sessionSys.GetEnemies();
    if (!allies || !enemies) return out;

    auto push = [&](EntityID entity) { if (entity != invalidEntity && Alive(entity)) out.push_back(entity); };

    if (team == BattleTeam::Ally)
        for (int i = 0; i < enemies->memberCount; ++i) push(enemies->members[i]);
    else
        for (int i = 0; i < allies->memberCount; ++i)  push(allies->members[i]);
    return out;
}

EntityID BattleTargetSystem::Pick(const vector<EntityID>& vec)
{
    if (vec.empty()) return invalidEntity;
    uniform_int_distribution<size_t> dist(0, vec.size() - 1);
    return vec[dist(rng)];
}

void BattleTargetSystem::Pair()
{
    auto& sessionSys = registry.Get<BattleSessionSystem>();
    const auto* state = sessionSys.TryGetState();
    if (!state) return;

    vector<int> allyOrder(state->allies.memberCount);
    iota(allyOrder.begin(),  allyOrder.end(),  0);
    vector<int> enemyOrder(state->enemies.memberCount);
    iota(enemyOrder.begin(), enemyOrder.end(), 0);

    shuffle(allyOrder.begin(),  allyOrder.end(), rng);
    shuffle(enemyOrder.begin(), enemyOrder.end(), rng);

    const int pairCount = static_cast<int>(min(allyOrder.size(), enemyOrder.size()));
    for (int i = 0; i < pairCount; ++i)
    {
        EntityID ally  = state->allies.members[allyOrder[i]];
        EntityID enemy = state->enemies.members[enemyOrder[i]];
        if (ally  != invalidEntity) Set(ally, enemy);
        if (enemy != invalidEntity) Set(enemy, ally);
    }

    if (allyOrder.size() > static_cast<size_t>(pairCount))
    {
        vector<int> rest(allyOrder.begin() + pairCount, allyOrder.end());
        FillRest(rest, BattleTeam::Ally);
    }
    if (enemyOrder.size() > static_cast<size_t>(pairCount))
    {
        vector<int> rest(enemyOrder.begin() + pairCount, enemyOrder.end());
        FillRest(rest, BattleTeam::Enemy);
    }
}

void BattleTargetSystem::FillRest(const vector<int>& order, BattleTeam team)
{
    auto& sessionSys = registry.Get<BattleSessionSystem>();
    const auto* state = sessionSys.TryGetState();
    if (!state) return;

    vector<EntityID> pool = Opponents(team);
    for (int idx : order)
    {
        EntityID self = (team == BattleTeam::Ally) ? ((idx < state->allies.memberCount)  ? state->allies.members[idx]  : invalidEntity)
                                                   : ((idx < state->enemies.memberCount) ? state->enemies.members[idx] : invalidEntity);
        if (self == invalidEntity) continue;
        Ensure(self);
        Set(self, Pick(pool));
    }
}

void BattleTargetSystem::Set(EntityID attacker, EntityID target)
{
    if (attacker == invalidEntity) return;
    Target* t = GetByOwner(attacker);
    if (!t)
    { 
        CreateComp(attacker);
        t = GetByOwner(attacker);
    }
    if (t)
        t->curTarget = target;
}

void BattleTargetSystem::RenderGui(EntityID owner)
{
#ifdef USE_IMGUI
    if (owner == invalidEntity) return;

    ForEachOwned(owner, [&](Handle handle, Target& tgt)
        {
            ImGui::PushID((int)handle.idx);

            const ImGuiTreeNodeFlags flags =  ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowItemOverlap;

            if (ImGui::TreeNodeEx("Target", flags))
            {
                auto& session = registry.Get<BattleSessionSystem>();
                BattleTeam myTeam{};
                const bool hasTeam = session.TryGetTeam(owner, myTeam);

                vector<EntityID> candidates;
                if (hasTeam) candidates = Opponents(myTeam);

                ImGui::Text("Owner: %u", owner);
                ImGui::Text("Current Target: %u", tgt.curTarget);

                string curLabel = (tgt.curTarget == invalidEntity)  ? string("None")  : to_string(tgt.curTarget);

                if (ImGui::BeginCombo("Retarget To", curLabel.c_str()))
                {
                    bool pickedNone = (tgt.curTarget == invalidEntity);
                    if (ImGui::Selectable("None", pickedNone))
                        tgt.curTarget = invalidEntity;
                    if (pickedNone) ImGui::SetItemDefaultFocus();

                    for (EntityID cand : candidates)
                    {
                        const bool selected = (tgt.curTarget == cand);
                        string label = to_string(cand);
                        if (ImGui::Selectable(label.c_str(), selected))
                            tgt.curTarget = cand;
                        if (selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                if (ImGui::Button("Pick Random"))
                    tgt.curTarget = Pick(candidates);
                ImGui::SameLine();
                if (ImGui::Button("Ensure Component"))
                    Ensure(owner);

                if (!hasTeam)
                    ImGui::TextDisabled("Team: Unknown (opponents list unavailable)");
                else
                    ImGui::Text("Opponents: %zu", candidates.size());

                ImGui::TreePop();
            }
            ImGui::PopID();
        });
#endif
}