#pragma once

NS_BEGIN(Client)

class SoundLoader
{
public:
	static void Load(SoundRegistry* sound);

private:
	static void LoadRyzaSound(SoundRegistry* sound);
	static void LoadKlaudiaSound(SoundRegistry* sound);
	static void LoadPatriciaSound(SoundRegistry* sound);
	static void LoadBGM(SoundRegistry* sound);
	static void LoadEffectSound(SoundRegistry* sound);
};

NS_END