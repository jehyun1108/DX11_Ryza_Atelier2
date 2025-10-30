#pragma once

NS_BEGIN(Engine)

struct FacingParams
{
    float forwardOffsetRad = 0.f;
    float turnSpeed        = 720.f;
};

struct FacingComponent
{
    FacingParams params{};
};

NS_END