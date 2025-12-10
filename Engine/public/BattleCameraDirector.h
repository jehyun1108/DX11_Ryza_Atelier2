#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL BattleCameraDirector : public ISystem
{
public:
	using SeqSampleFunc = function<bool(ClipId clip, double tLocal, const SequenceTrackDesc& desc, CamPose& outLocalPose)>;

	explicit BattleCameraDirector(SystemRegistry& registry);
	void     OnBoot() override;

	void     BindCam(Handle camHandle);
	void     SetSequenceSampler(SeqSampleFunc func)    { seqSampler = move(func); }
	void     SetSmoothing(const SmoothingConfig& cfg)  { smooth = cfg; }
	void     SetFixedLens(const Lens& lens);
	Lens     GetFixedLens() const { return state.fixedLens; }

	TrackID Spawn(const TrackSpawnRequest& req);
	void    Kill(const TrackKillRequest& req);

	void    SetAnchor(TrackID id, const AnchorBinding& anchor);
	void    SetPriority(TrackID id, CamPriority priority);
	void    SetLayer(TrackID id, CamLayer layer);
	void    SetGoal(TrackID id, const CamPose& goal);
	void    SetSeqClips(TrackID id, const vector<ShotClip>& clips);
	void    SnapTrackToPose(TrackID id, const CamPose& pose);
	void    SetSeqDesc(TrackID id, const SequenceTrackDesc& desc);

	void    SetDebugCam(Handle tf);
	void    ClearDebugCam();
	
	void    Tick(float dt);
	CamPose GetOutput() const { return state.output; }

	FollowTrackDesc& GetFollowDesc(TrackID id);
	bool HasActiveSequenceTrack() const;
	bool GetTopSequenceLens(Lens& outLens) const;
	void SnapTrackToOutput(TrackID id);

private:
	void     RebuildGroups();
	void     AdvanceTracks(float dt);
	void     AdvanceFollow(TrackState& track, float dt);
	void     AdvanceSequence(TrackState& track, float dt);
	void     ApplyAnchors(TrackState& track, const CamPose* localOpt = nullptr);                

	CamPose  MixByGroups(const CamPose& prev) const;
	CamPose  MixGroup(const MixerGroup& group) const;
	CamPose  MixLayered(const vector<const TrackState*>& base, const vector<const TrackState*>& action, const vector<const TrackState*>& overlay) const;

	TrackState*       Find(TrackID id);
	const TrackState* Find(TrackID id) const;
	TrackID           AllocID();
	EntityID          ResolveAnchorEntity(const AnchorBinding& anchor) const;
	bool              GetEntityWorldPos(EntityID entity, _vec& outPos, _vec& outRot) const;
	bool              ComputeFollowTarget(const TrackState& track, _vec& outTargetPos) const;
	TrackState&       RequireTrack(TrackID id);

private:
	DirectorState               state;
	SmoothingConfig             smooth{};
	Handle                      cam{};
	_uint                       nextIdx = 1;
	unordered_map<_uint, _uint> gen;
	SeqSampleFunc               seqSampler;
	bool                        debugCamActive = false;
	Handle                      debugCamTf{};

private:
	SystemRegistry&             registry;
	BattleTimelineSystem*       timelineSys{};
	BattleTargetSystem*         targetSys{};
	TransformSystem*            tfSys{};
	CameraSystem*               camSys{};
};

NS_END