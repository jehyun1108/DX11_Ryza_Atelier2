#include "Enginepch.h"
#include "SoundRegistry.h"
#include "fmod/fmod_studio.hpp"
#include "fmod/fmod_errors.h"
#include "fmod/fmod.hpp"

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

void SoundRegistry::Init(FMOD::System* s)
{
    core = s;
    assert(core);
}

void SoundRegistry::Register(const wstring& key, const wstring& path, bool stream)
{
    assert(core);

    auto it = sounds.find(key);
    assert(it == sounds.end());

    FMOD::Sound* snd = nullptr;
    FMOD_MODE mode = FMOD_DEFAULT;
    if (stream)
        mode |= FMOD_CREATESTREAM;

    FMOD_CHECK(core->createSound(Utility::ToString(path).c_str(), mode, nullptr, &snd));
    assert(snd);

    Entry e;
    e.sound = snd;
    e.stream = stream;

    sounds.emplace(key, e);
}

FMOD::Sound* SoundRegistry::Get(const wstring& key)
{
    auto it = sounds.find(key);
    assert(it != sounds.end());
    return it->second.sound;
}

void SoundRegistry::Unload(const wstring& key)
{
    auto it = sounds.find(key);
    assert(it != sounds.end());

    Entry& e = it->second;
    if (e.sound)
    {
        FMOD_CHECK(e.sound->release());
        e.sound = nullptr;
    }

    sounds.erase(it);
}

void SoundRegistry::UnloadAll()
{
    for (auto& kv : sounds)
    {
        Entry& e = kv.second;
        if (!e.sound) continue;
        FMOD_CHECK(e.sound->release());
        e.sound = nullptr;
    }
    sounds.clear();
}