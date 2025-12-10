#pragma once

NS_BEGIN(Engine)

struct SoundSystemInit
{
    int maxChannels = 128;
    int sampleRate = 48000;
    int dspBufferLength = 1024;
    int dspNumBuffers = 4;
};

enum class PlayMode
{
    Once,
    Loop,
};

using VoiceID = _uint;

NS_END