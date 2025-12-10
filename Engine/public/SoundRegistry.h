#pragma once

#include "fmod/fmod_studio.hpp"
#include "fmod/fmod_errors.h"
#include "fmod/fmod.hpp"

NS_BEGIN(Engine)

class ENGINE_DLL SoundRegistry : public ISystem
{
public:
    explicit SoundRegistry(SystemRegistry& registry) : registry(registry) {}

    void Init(FMOD::System* core);

    void Register(const wstring& key, const wstring& path, bool stream = false);
    FMOD::Sound* Get(const wstring& key);

    void Unload(const wstring& key);
    void UnloadAll();

private:
    struct Entry
    {
        FMOD::Sound* sound = nullptr;
        bool         stream = false;
    };

private:
    SystemRegistry& registry;

    FMOD::System* core = nullptr;
    unordered_map<wstring, Entry> sounds;
};

NS_END