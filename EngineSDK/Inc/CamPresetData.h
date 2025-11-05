#pragma once

NS_BEGIN(Engine)

enum class BattleCamKey
{
	Default_Follow, Intro_Action, Skill_Action,
};

struct TrackPreset
{
	CamTrackType      type;
	CamPriority       priority;
	CamLayer          layer;

	AnchorBinding     anchor;
	CamBlendConfig    blendCfg;
	FollowTrackDesc   followDesc;
	SequenceTrackDesc seqDesc;
};

NS_END