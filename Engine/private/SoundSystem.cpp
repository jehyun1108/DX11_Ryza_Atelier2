#include "Enginepch.h"
#include "SoundSystem.h"
#include "SoundRegistry.h"

// =====================================
// ===============================================================
using std::vector;
static inline FMOD_VECTOR ToFMOD(const _float3& v) { return FMOD_VECTOR{ v.x, v.y, v.z }; }

#ifndef FMOD_CHECK
#define FMOD_CHECK(expr)                                                     \
    do {                                                                     \
        FMOD_RESULT _r = (expr);                                             \
        if (_r != FMOD_OK) {                                                 \
            char _buf[512];                                                  \
            _snprintf_s(_buf, sizeof(_buf),                                  \
                "FMOD ERROR %d: %s\nat %s(%d)\nexpr: %s\n",                  \
                (int)_r, FMOD_ErrorString(_r), __FILE__, __LINE__, #expr);   \
            OutputDebugStringA(_buf);                                        \
            __debugbreak();                                                  \
        }                                                                    \
    } while (0)
#endif

// ===========================================================================================
void SoundSystem::Init()
{
    SoundSystemInit cfg;

    soundRegistry = &registry.Get<SoundRegistry>();

    FMOD::System* s = nullptr;
    FMOD_CHECK(FMOD::System_Create(&s));
    core = s;

    FMOD_CHECK(core->setSoftwareFormat(cfg.sampleRate, FMOD_SPEAKERMODE_DEFAULT, 0));
    FMOD_CHECK(core->setDSPBufferSize(cfg.dspBufferLength, cfg.dspNumBuffers));

    FMOD_CHECK(core->init(cfg.maxChannels, FMOD_INIT_NORMAL, nullptr));

    FMOD_CHECK(core->createChannelGroup("Master", &masterGroup));
    FMOD_CHECK(core->createChannelGroup("BGM", &bgmGroup));
    FMOD_CHECK(core->createChannelGroup("SFX", &sfxGroup));

    FMOD_CHECK(masterGroup->addGroup(bgmGroup));
    FMOD_CHECK(masterGroup->addGroup(sfxGroup));

    soundRegistry->Init(core);

    SetMasterVolume(masterVolume);
    SetBgmVolume(bgmVolume);
    SetSfxVolume(sfxVolume);
}

void SoundSystem::Shutdown()
{
    voices.clear();
    bgmChannel = nullptr;

    if (masterGroup)
    {
        masterGroup->release();
        masterGroup = nullptr;
        bgmGroup = nullptr;
        sfxGroup = nullptr;
    }

    if (core)
    {
        core->close();
        core->release();
        core = nullptr;
    }
}

void SoundSystem::Tick(float dt)
{
    if (!pendingPlays.empty())
    {
        for (auto& p : pendingPlays)
            p.remaining -= dt;

        vector<size_t> fired;
        fired.reserve(pendingPlays.size());

        for (size_t i = 0; i < pendingPlays.size(); ++i)
        {
            if (pendingPlays[i].remaining <= 0.f)
            {
                const auto& p = pendingPlays[i];
                Play(p.key, p.volume, p.mode);
                fired.push_back(i);
            }
        }
        for (size_t i = fired.size(); i > 0; --i)
        {
            size_t idx = fired[i - 1];
            pendingPlays.erase(pendingPlays.begin() + idx);
        }
    }

    vector<VoiceID> dead;

    for (auto& kv : voices)
    {
        VoiceID id = kv.first;
        FMOD::Channel* ch = kv.second;

        bool playing = false;
        FMOD_RESULT r = ch->isPlaying(&playing);

        if (r != FMOD_OK || !playing)
            dead.push_back(id);
    }

    for (VoiceID id : dead)
        voices.erase(id);

    if (core)
        FMOD_CHECK(core->update());
}

VoiceID SoundSystem::Play(const wstring& key, float volume, PlayMode mode)
{
    assert(core);
    assert(soundRegistry);

    FMOD::Sound* snd = soundRegistry->Get(key);
    assert(snd);

    FMOD::Channel* ch = nullptr;
    FMOD_CHECK(core->playSound(snd, sfxGroup, false, &ch));
    assert(ch);

    if (mode == PlayMode::Loop)
        FMOD_CHECK(ch->setMode(FMOD_LOOP_NORMAL));
    else
        FMOD_CHECK(ch->setMode(FMOD_LOOP_OFF));

    FMOD_CHECK(ch->setVolume(volume));

    VoiceID id = NewVoiceID();
    voices[id] = ch;
    return id;
}

void SoundSystem::PlayAfter(const wstring& key, float delayDur, float volume, PlayMode mode)
{
    if (delayDur <= 0.f)
    {
        Play(key, volume, mode);
        return;
    }

    PendingPlay p;
    p.key = key;
    p.mode = mode;
    p.volume = volume;
    p.remaining = delayDur;

    pendingPlays.push_back(move(p));
}

VoiceID SoundSystem::PlaySkipDur(const wstring& key, float skipDur, float volume, PlayMode mode)
{
    if (skipDur <= 0.f)
        return Play(key, volume, mode);

    assert(core);
    assert(soundRegistry);

    FMOD::Sound* snd = soundRegistry->Get(key);
    assert(snd);

    unsigned int lengthMs = 0;
    FMOD_CHECK(snd->getLength(&lengthMs, FMOD_TIMEUNIT_MS));

    unsigned int skipMs = static_cast<unsigned int>(skipDur * 1000.f);
    assert(skipMs < lengthMs); // 데이터 없으면 터져야 한다 원칙

    FMOD::Channel* ch = nullptr;

    FMOD_CHECK(core->playSound(snd, sfxGroup, true, &ch));
    assert(ch);

    if (mode == PlayMode::Loop)
        FMOD_CHECK(ch->setMode(FMOD_LOOP_NORMAL));
    else
        FMOD_CHECK(ch->setMode(FMOD_LOOP_OFF));

    FMOD_CHECK(ch->setVolume(volume));
    FMOD_CHECK(ch->setPosition(skipMs, FMOD_TIMEUNIT_MS));
    FMOD_CHECK(ch->setPaused(false));

    VoiceID id = NewVoiceID();
    voices[id] = ch;
    return id;
}

void SoundSystem::Stop(VoiceID id)
{
    auto it = voices.find(id);
    assert(it != voices.end());

    FMOD::Channel* ch = it->second;
    if (ch)
        FMOD_CHECK(ch->stop());

    voices.erase(it);
}

bool SoundSystem::IsPlaying(VoiceID id) const
{
    auto it = voices.find(id);
    if (it == voices.end())
        return false;

    FMOD::Channel* ch = it->second;

    bool playing = false;
    FMOD_RESULT r = ch->isPlaying(&playing);

    if (r != FMOD_OK)
        return false;

    return playing;
}

void SoundSystem::PlayBgm(const wstring& key, float volume)
{
    assert(core);
    assert(soundRegistry);

    FMOD::Sound* snd = soundRegistry->Get(key);
    assert(snd);

    if (bgmChannel)
        FMOD_CHECK(bgmChannel->stop());

    FMOD::Channel* ch = nullptr;
    FMOD_CHECK(core->playSound(snd, bgmGroup, false, &ch));
    assert(ch);

    FMOD_CHECK(ch->setMode(FMOD_LOOP_NORMAL));
    FMOD_CHECK(ch->setVolume(volume));

    bgmChannel = ch;
    bgmVolume = volume;
}

void SoundSystem::StopBgm()
{
    if (!bgmChannel)
        return;

    FMOD_CHECK(bgmChannel->stop());
    bgmChannel = nullptr;
}

bool SoundSystem::IsBgmPlaying() const
{
    if (!bgmChannel)
        return false;

    bool playing = false;
    FMOD_CHECK(bgmChannel->isPlaying(&playing));
    return playing;
}

void SoundSystem::SetMasterVolume(float v)
{
    masterVolume = v;
    if (masterGroup)
        FMOD_CHECK(masterGroup->setVolume(v));
}

void SoundSystem::SetBgmVolume(float v)
{
    bgmVolume = v;
    if (bgmGroup)
        FMOD_CHECK(bgmGroup->setVolume(v));
}

void SoundSystem::SetSfxVolume(float v)
{
    sfxVolume = v;
    if (sfxGroup)
        FMOD_CHECK(sfxGroup->setVolume(v));
}
