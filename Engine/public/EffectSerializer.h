#pragma once

NS_BEGIN(Engine)

enum class EffectFileVersion : _uint
{
	V1 = 1, 
	V2 = 2,
	V3 = 3,
};
constexpr _uint EFFECT_VERSION = static_cast<_uint>(EffectFileVersion::V3);

class ENGINE_DLL EffectSerializer : public ISystem
{
public:
	explicit EffectSerializer(SystemRegistry& registry) : registry(registry) {}

	bool Save(const EffectArchetype& effect, const filesystem::path& path);
	bool Load(EffectArchetype& out, const filesystem::path& path);

private:
	bool WriteWString(ofstream& outFile, const wstring& wstr);
	bool ReadWString(ifstream& inFile, wstring& out);

private:
	bool LoadV2(EffectArchetype& out, ifstream& in);
	bool LoadV3(EffectArchetype& out, ifstream& in);

	void WriteParticleV3(ofstream& out, const ParticleSpawnData& sd);
	bool ReadParticleV3(ifstream& in, ParticleSpawnData& sd);

private:
	SystemRegistry& registry;
};

NS_END