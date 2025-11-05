#pragma once

#include "CamRegistryData.h"

NS_BEGIN(Engine)

class ENGINE_DLL CamRegistry
{
public:
	explicit CamRegistry(SystemRegistry& registry) : registry(registry) {}

	void BindDirector(BattleCameraDirector& director);
	void BindCam(Handle cam);
	void SetSequenceSampler(BattleCameraDirector::SeqSampleFunc func);

	void RegisterDefaults();

	TrackID SpawnDefaultToFollow();
	TrackID SpawnIntro();
	void    KillRecent(float outDur = 0.25f);
	void    ClearAll() { recent.clear(); }

private:
	bool    Build(BattleCamKey key, TrackSpawnRequest& out) const;
	TrackID Spawn(const TrackSpawnRequest& req, bool trackRecent);

private:
	SystemRegistry&                 registry;
	BattleCameraDirector*           director{};
	unordered_map<int, TrackPreset> presets;
	vector<TrackID>                 recent;

	unordered_map<ClipId, BattleCameraDirector::SeqSampleFunc> samplers;
	unordered_map<ClipId, vector<ShotClip>> introClips;

	TrackID baseFollowId{};
};

NS_END