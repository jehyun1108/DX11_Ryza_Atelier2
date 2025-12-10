#pragma once

#include "CamRegistryData.h"

NS_BEGIN(Engine)
class BattleCameraDirector;

class ENGINE_DLL CamRegistry : public ISystem, public IGuiRenderable
{
public:
	explicit CamRegistry(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void     BindDirector();
	void     BindCam(Handle cam) { director->BindCam(cam); }

	void     RegisterSeqClips(ClipId clipId, const vector<ShotClip>& clips) { introClips[clipId] = clips; }
	void     RegisterDefaults();
			 
	TrackID  SpawnDefaultToFollow();
	TrackID  SpawnIntro();
	void     KillRecent(float outDur = 0.25f);
	void     ClearAll() { recent.clear(); }

	TrackID  Spawn(const TrackSpawnRequest& req, bool trackRecent);
	void     SetDebugCam(Handle cam, Handle tf);
	
	void     RenderGui(EntityID id) override;
	bool     ComputeFocusPos(_float3& outLeaderPos, _float3& outFocusPos, _vec& outRight, _vec& outUp, _vec& outForward) const;
	ClipId   RegisterCamClip(const filesystem::path& path);

	ClipId   FindBasicAttackCam(CharacterID characterId) const;
	void     StopActionTrack(TrackID id);
	void     SmoothBackToFollow();
	static float ComputeCamDuration(const SeqCamPreset& preset);

	ClipId  FindRewardCam(CharacterID characterId) const;
	TrackID PlayRewardCam(CharacterID characterId, EntityID entity);

	bool    ComputeFocusPosFor(EntityID leader, _float3& outLeaderPos, _float3& outFocusPos, _vec& outRight, _vec& outUp, _vec& outForward) const;

	CamFadeProfile GetFadeProfile(CamRole role) const;

private:
	bool     Build(BattleCamKey key, TrackSpawnRequest& out) const;
	void     EvalSeqEvent(ClipId clip, const SeqCamPreset& preset, float t, CamPose& outLocal) const;
	float    EaseInOut(float x) const;
	void     CaptureKeyFromCamera(SeqCamPreset& preset, size_t keyIdx);
	_float3  EvalCatmullRom(const _float3& p0, const _float3& p1, const _float3& p2, const _float3& p3, float u) const;

private:
	unordered_map<int, TrackSpawnRequest>                      presets;
	vector<TrackID>                                            recent;
	unordered_map<ClipId, BattleCameraDirector::SeqSampleFunc> samplers;
	unordered_map<ClipId, vector<ShotClip>>                    introClips;
	unordered_map<ClipId, EntityID> seqBasisEntity;

	TrackID baseFollowId{};
	TrackID actionTrackId{};
	
	unordered_map<ClipId, SeqCamPreset>  seqPresets;
	unordered_map<CharacterID, ClipId>   basicAttackIdx;
	unordered_map<CharacterID, ClipId>   rewardIdx;

	Handle                debugCamTf{};
	Handle                debugCamCam{};
	Handle                prevMainCam{};
	bool                  debugCamOverride = false;

private:
	SystemRegistry&       registry;
	BattleCameraDirector* director{};
	CameraSystem*         camSys{};
	TransformSystem*      tfSys{};
	CamSerializer*        camSerializer{};
	BattleTargetSystem*   targetSys{};
	BattleTimelineSystem* timelineSys{};
	ActionCamRegistry*    actionCamReg{};
	ScreenFadeSystem*     fadeSys{};
};

NS_END