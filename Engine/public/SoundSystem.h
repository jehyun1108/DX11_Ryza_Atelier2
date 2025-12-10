#pragma once

#include "fmod/fmod_studio.hpp"
#include "fmod/fmod_errors.h"
#include "fmod/fmod.hpp"

#include "SoundData.h"

NS_BEGIN(Engine)
class SoundRegistry;

class ENGINE_DLL SoundSystem : public ISystem
{
public:
    explicit SoundSystem(SystemRegistry& registry) : registry(registry) {}

    void    Init();
    void    Shutdown();
    void    Tick(float dt);

    VoiceID Play(const wstring& key, float volume = 0.5f, PlayMode mode = PlayMode::Once);
    void    PlayAfter(const wstring& key, float delayDur, float volume = 0.5f, PlayMode mode = PlayMode::Once);
    VoiceID PlaySkipDur(const wstring& key, float skipDur, float volume = 0.5f, PlayMode mode = PlayMode::Once);

    void    Stop(VoiceID id);
    bool    IsPlaying(VoiceID id) const;

    void    PlayBgm(const wstring& key, float volume = 0.2f);
    void    StopBgm();
    bool    IsBgmPlaying() const;

    void    SetMasterVolume(float v);
    void    SetBgmVolume(float v);
    void    SetSfxVolume(float v);

    float   GetMasterVolume() const { return masterVolume; }
    float   GetBgmVolume()    const { return bgmVolume; }
    float   GetSfxVolume()    const { return sfxVolume; }

private:
    VoiceID NewVoiceID() { return nextId++; }

private:
    struct PendingPlay
    {
        wstring  key;
        PlayMode mode;
        float    volume;
        float    remaining;
    };


    SystemRegistry& registry;

    FMOD::System* core = nullptr;
    SoundRegistry* soundRegistry = nullptr;

    FMOD::ChannelGroup* masterGroup = nullptr;
    FMOD::ChannelGroup* bgmGroup = nullptr;
    FMOD::ChannelGroup* sfxGroup = nullptr;

    FMOD::Channel* bgmChannel = nullptr;

    unordered_map<VoiceID, FMOD::Channel*> voices;
    VoiceID nextId = 1;

    vector<PendingPlay> pendingPlays;

    float masterVolume = 1.f;
    float bgmVolume = 1.f;
    float sfxVolume = 1.f;
};

NS_END