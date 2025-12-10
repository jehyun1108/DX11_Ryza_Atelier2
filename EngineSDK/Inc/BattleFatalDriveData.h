#pragma once

NS_BEGIN(Engine)

struct FatalDriveKeys
{
	wstring ringBack    = L"fataldrive_barback";
	wstring ringFront   = L"fataldrive_barfront";
	wstring portrait    = L"patricia_fataldrive";
	wstring txtFatal    = L"fataldrive_fatal";
	wstring txtRedLight = L"fataldrive_redlight";
	wstring txtDrive    = L"fataldrive_drive";
	wstring bgLeft      = L"fataldrive_bg_left";
	wstring bgRight     = L"fataldrive_bg_right";
};

struct FatalDriveLayout
{
	_float2 pos{ 0.f, 0.f };
	_float2 scale{ 1.f, 1.f };
};

struct FatalDriveConfig
{
	FatalDriveKeys keys;
	FatalDriveLayout layout;
};
// 글자 연출 설정
struct LetterRevealSpec
{
	vector<wstring> keys;
	float           revealDur   = 0.2f;
	float           staggerFrac = 0.5f;
	float           driftX      = -100.f;
	float           driftDur    = 2.f;
	UIEasing        inEase      = UIEasing::EaseOutCubic;
	UIEasing        outEase     = UIEasing::EaseOutCubic;
	float           outDur      = 0.70f;
	float           outScale    = 1.20f;
};
// 글자 연출 런타임
struct LetterRevealRt
{
	bool  active = false;
	float elapsed = 0.f;          
	float stagger = 0.f;          
	vector<uint8_t> started;     

	void Reset(size_t count, float revealDur, float staggerFrac)
	{
		active = false;
		elapsed = 0.f;
		stagger = revealDur * staggerFrac;
		started.assign(count, 0);
	}
};

struct FatalDriveRuntime
{
	bool           active            = false;
	float          progress          = 0.f;
	bool           portraitShown     = false;
	bool           portraitFadingOut = false;
	float          portraitTimer     = 0.f;
	LetterRevealRt letters;
};

NS_END