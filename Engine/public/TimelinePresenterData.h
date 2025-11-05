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

    int   zBaseAllies          = 12000;
    int   zBaseEnemies         = 12000;
    int   zBiasLeaderBonus     = 1000;
    int   zBiasProgressScale   = 100;
    int   zBiasTieBreakerStep  = 1;

    float defaultIconScale = 0.8f;
    float leaderIconScale  = 1.f;

    float scaleAnimInDur   = 0.15f;
    float scaleAnimOutDur  = 0.10f;
};

NS_END