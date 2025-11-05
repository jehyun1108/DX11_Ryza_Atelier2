#pragma once

#include "BattleCamera_Enum.h"
#include "BattleCamera_Struct.h"

NS_BEGIN(Engine)

class ENGINE_DLL BattleCameraDirector
{
public:
	using SeqSampleFunc = function<bool(ClipId clip, double tLocal, const SequenceTrackDesc& desc, CamPose& outLocalPose)>;

	explicit BattleCameraDirector(SystemRegistry& registry);

	void     BindCam(Handle camHandle)                { cam = camHandle; }
	void     SetSequenceSampler(SeqSampleFunc func)    { seqSampler = move(func); }
	void     SetSmoothing(const SmoothingConfig& cfg) { smooth = cfg; }
	void     SetFixedLens(const Lens& lens);

	TrackID Spawn(const TrackSpawnRequest& req);
	bool    Kill(const TrackKillRequest& req);

	bool    SetAnchor(TrackID id, const AnchorBinding& anchor);
	bool    SetPriority(TrackID id, CamPriority priority);
	bool    SetLayer(TrackID id, CamLayer layer);
	bool    SetGoal(TrackID id, const CamPose& goal);
	bool    SetSeqClips(TrackID id, const vector<ShotClip>& clips);
	
	void    Tick(float dt);
	CamPose GetOutput() const { return state.output; }

private:
	void     RebuildGroups();
	void     AdvanceTracks(float dt);
	void     AdvanceFollow(TrackState& track, float dt);
	void     AdvanceSequence(TrackState& track, float dt);
	void     ApplyAnchors(TrackState& track, const CamPose* localOpt = nullptr);

	void     ApplyTrackSmoothing(TrackState& track, float dt);     
	void     SnapIfClose(TrackState& track);                    
	CamPose  ClampStep(const CamPose& prev, const CamPose& next, float dt) const;
	
	CamPose  MixByGroups(const CamPose& prev) const;
	CamPose  MixGroup(const MixerGroup& group) const;
	CamPose  MixLayered(const vector<const TrackState*>& base, const vector<const TrackState*>& action, const vector<const TrackState*>& overlay) const;

	TrackState*       Find(TrackID id);
	const TrackState* Find(TrackID id) const;
	TrackID           AllocID();
	EntityID          ResolveAnchorEntity(const AnchorBinding& anchor) const;
	bool              GetEntityWorldPos(EntityID entity, _vec& outPos, _vec& outRot) const;
	bool              ComputeFollowTarget(const TrackState& track, _vec& outTargetPos) const;

private:
	SystemRegistry&             registry;
	DirectorState               state;
	SmoothingConfig             smooth{};
	Handle                      cam{};
	_uint                       nextIdx = 1;
	unordered_map<_uint, _uint> gen;
	SeqSampleFunc               seqSampler;
};

NS_END