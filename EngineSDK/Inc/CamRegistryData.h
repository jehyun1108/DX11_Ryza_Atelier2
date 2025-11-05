#pragma once

NS_BEGIN(Engine)

enum class BattleCamKey
{
	Default_Follow, Intro_Action
};

struct TrackPreset
{
	CamTrackType      type;
	CamPriority       priority;
	CamLayer          layer;
	AnchorBinding     anchor;
	FollowTrackDesc   followDesc;
	SequenceTrackDesc seqDesc;
};

NS_END