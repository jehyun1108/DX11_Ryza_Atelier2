#pragma once

#include "BattleDamageData.h"

NS_BEGIN(Engine)

class ENGINE_DLL BattleDamagePresenter : public ISystem
{
public:
	explicit BattleDamagePresenter(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void     Enter();
	void     Tick(float dt);

	void     SetFont(const DamageFontSet& set) { font = set; }
	void     SetSpec(const DamageHudSpec& s)   { spec = s; }
	
	void     SpawnDamage(int dmg);
	void     Wire();
	void     UnWire();
	void     SetChain(int chainValue);

private:
	void     EnsureDigit(const wstring& key, const wstring& instKey);
	void     BuildDigits(int value, vector<wstring>& outKeys, _uint spawnId);
	void     AnimateIn(const vector<wstring>& keys, float x, float y);
	void     AnimateOut(const vector<wstring>& keys);

	void     RelayoutRightAligned(DamageNumber& num);
	void     SetDigitsTextures(DamageNumber& num, int value);
	void     AnimateOutTotal();

private:
	DamageFontSet        font{};
	DamageHudSpec        spec{};
	DamageFontSet        chainFont{};
	ChainHUD             chain{};
	DamageNumber         total{};

	vector<DamageNumber> actives;
	bool                 wired = false;
	vector<_uint>        listenerIds;
	_uint                spawnSerial = 0;
	float                countPerSec = 60.f;

	struct Burst
	{
		int   sum       = 0;
		int   critCount = 0;
		float timer     = 0.f;
		bool  active    = false;
	};

	Burst     accum{};
	float     accumWindow   = 0.16f;
	float     marginRight   = 120.f;
	float     anchorYOffset = 0.f;

	deque<int> queue;
	bool       playing = false;

private:
	SystemRegistry&       registry;
	UIRegistry*           uiRegistry{};
	UIAnimSystem*         uiAnimSys{};
	BattleEventBus*       eventBus{};
	BattleTimelineSystem* timelineSys{};
	BattleSessionSystem*  sessionSys{};
	SoundSystem*          soundSys{};
};

NS_END