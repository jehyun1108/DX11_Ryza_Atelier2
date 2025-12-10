#pragma once

#include "CamBinaryData.h"

NS_BEGIN(Engine)

enum class CamFileVersion : _uint
{
	V1 = 1, V2 = 2
};

class ENGINE_DLL CamSerializer : public ISystem
{
public:
	CamSerializer(SystemRegistry& registry) : registry(registry) {}

	void OnBoot() override;

	bool Save(ClipId clipId, const SeqCamPreset& preset, float baseFovDeg, const FollowTrackDesc& follow, const filesystem::path& path);
	bool Load(ClipId& outClipId, SeqCamPreset& outPreset, float& outBaseFovDeg, FollowTrackDesc& outFollow, const filesystem::path& path);

private:
	SystemRegistry& registry;
	CamRegistry*    camReg{};
};

NS_END