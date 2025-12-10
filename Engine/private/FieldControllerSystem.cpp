#include "Enginepch.h"
#include "WorldMapPresenter.h"
#include "DressingRoomPresenter.h"
#include "WorldMapPresenter.h"

void FieldControllerSystem::OnBoot()
{
    tfSys     = &registry.Get<TransformSystem>();
    moveSys   = &registry.Get<MoveStateSystem>();
    input     = &registry.Get<InputService>();
    worldMap  = &registry.Get<WorldMapPresenter>();
    fieldUI   = &registry.Get<FieldUIOrchestrator>();
    effectSys = &registry.Get<EffectSystem>();
    dressing  = &registry.Get<DressingRoomPresenter>();
}

Handle FieldControllerSystem::Create(EntityID leader, Handle camTf)
{
    Handle handle       = CreateComp(leader);
    auto& data          = *Get(handle);
    data.leader         = leader;
    data.camTf          = camTf;
    data.prevJumpDown   = false;
    data.prevAttackDown = false;
    return handle;
}

void FieldControllerSystem::Update(EntityID leader, float dt)
{
    auto state = GetByOwner(leader);
    if (!state) return;

    if (dressing->IsActive())
    {
        state->prevJumpDown = false;
        state->prevAttackDown = false;
        return;
    }

    if (worldMap->HasTeleport())
    {
        const _float3 dst = worldMap->ConsumeTeleport();
        const MoveState* moveState = moveSys->GetByOwner(leader);
        
        Handle leaderTf = moveState->tfHandle;
        tfSys->SetPos(leaderTf, dst);

        const wstring& spotName = worldMap->GetLastTeleportSpotName();
        fieldUI->ShowMapTitle(spotName);
    }

    SubmitFieldMoveIntent(leader, *state, dt);
}

void FieldControllerSystem::SubmitFieldMoveIntent(EntityID leader, FieldControllerState& state, float dt)
{
    const MoveState* moveState = moveSys->GetByOwner(leader);
    if (!moveState || worldMap->IsActive()) return;

    float localRight = 0.f;
    float localForward = 0.f;

    if (input->KeyPressing(KEY::D)) localRight   += 1.f;
    if (input->KeyPressing(KEY::A)) localRight   -= 1.f;
    if (input->KeyPressing(KEY::W)) localForward += 1.f;
    if (input->KeyPressing(KEY::S)) localForward -= 1.f;

    const PlanarBasisXZ basis = tfSys->GetPlanarBasisXZ(state.camTf);

    _float2 moveDir =
    {
        basis.rightXZ.x * localRight + basis.forwardXZ.x * localForward,
        basis.rightXZ.y * localRight + basis.forwardXZ.y * localForward
    };
    if (moveDir.x != 0.f || moveDir.y != 0.f)
        moveDir = Utility::Normalize(moveDir);

    const bool runHeld    = input->KeyPressing(KEY::LSHIFT) || input->KeyPressing(KEY::RSHIFT);
    const bool jumpHeld   = input->KeyPressing(KEY::SPACE);
    const bool jumpEdge   = (jumpHeld && !state.prevJumpDown);
    state.prevJumpDown = jumpHeld;
    if (jumpEdge)
        input->PushJumpEdge(leader, InputChannel::Manual);

    const bool attackHeld = input->KeyPressing(KEY::LBUTTON);
    const bool attackEdge = (attackHeld && !state.prevAttackDown);
    state.prevAttackDown = attackHeld;

    if (attackEdge)
        input->PushAttackEdge(leader, InputChannel::Manual);

    MoveIntent intent{};
    intent.moveDir       = moveDir;
    intent.turnInput     = 0.f;
    intent.isRunning     = runHeld;

    IntentWrite write{};
    write.target  = leader;
    write.channel = InputChannel::Manual;
    write.intent  = intent;

    input->Submit(write);
}