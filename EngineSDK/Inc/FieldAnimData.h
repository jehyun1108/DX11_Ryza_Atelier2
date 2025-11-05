#pragma once

NS_BEGIN(Engine)

enum class LocomotionState
{ 
    Idle,
    WalkStart, WalkLoop, WalkEnd, // walkspeed = 500.f runspeed = 800.f
    RunStart,  RunLoop,  RunEnd,
    JumpStart, JumpLoop, JumpEnd,
    FieldSwing
};

struct LocomotionAnim
{
    Handle animHandle{};
    _uint  layerIdx = 0;

    LocomotionState cur = LocomotionState::Idle;
    wstring         curClipName;
    float           stateElapsed = 0.f;

    bool  wasGroundedPrev   = true;

    AnimProfile profile{ CharacterID::Ryza, AnimContext::Field };
};

struct LocoParams
{
    float walkStartThreshold = 0.2f;
    float walkStopThreshold  = 0.15f; 
    float runStartThreshold  = 1.5f;
    float runStopThreshold   = 1.2f; 

    float fadeVeryShort = 0.12f;  
    float fadeShort     = 0.20f;
};

NS_END