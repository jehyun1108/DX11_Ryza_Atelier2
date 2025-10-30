#pragma once

NS_BEGIN(Engine)

struct FacingParams
{
    float minInputToFace   = 0.05f;
    float forwardOffsetRad = 0.f;
    float turnSpeed        = 720.f;
};

struct FacingState
{
    bool         useOverride = false;
    FacingParams params{};
};

NS_END