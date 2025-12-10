#pragma once

NS_BEGIN(Engine)

struct LaneLayout
{
	float xStart;
	float xReady;
	float yBase;
};
struct TimelinePresenterConfig
{
    float marginLeft      = 120.f;
    float marginRight     = 120.f;
    float laneYBase       = 0.f;
    float readyCenterBias = 0.f; 

    int   zBaseAllies          = 1000;
    int   zBaseEnemies         = 1000;
    int   zBiasLeaderBonus     = 800;
    int   zBiasProgressScale   = 100;
    int   zBiasTieBreakerStep  = 1;

    float defaultIconScale = 0.6f;
    float leaderIconScale  = 0.8f;

    float scaleAnimInDur   = 0.15f;
    float scaleAnimOutDur  = 0.10f;
};

NS_END