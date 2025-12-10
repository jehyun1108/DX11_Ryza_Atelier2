#pragma once

NS_BEGIN(Engine)

struct HUDLayout
{
	_float2 leaderPos{ -1050.f, 415.f };
	array<_float2, 2> partyPos{ _float2{ -1105.f, 180.f }, _float2{-1165.f, -10.f} };
};
struct HUDScales
{
	float leaderScale = 1.f;
	float partyScale  = 0.6f;
};
struct BarKeys
{
	wstring hpFront;
	wstring stunFront;
};
struct HUDKeys
{
	wstring leaderPortrait = L"ryza_battleui";
	array<wstring, 2> partyPortrait = { L"klaudia_battleui", L"patricia_battleui" };
	wstring tacticLevelup = L"tactic_levelup";
	wstring tacticBarBack = L"tactic_barback";
	wstring damageGlowLeft = L"dmgGlow_left";
	wstring damageGlowRight = L"dmgGlow_right";
};
struct TacticKeys  
{
	array<wstring, 5> front{ L"tactic_barfront1", L"tactic_barfront2", L"tactic_barfront3", L"tactic_barfront4", L"tactic_barfront5" };
	array<wstring, 5> full { L"tactic_barfull1" , L"tactic_barfull2" , L"tactic_barfull3" , L"tactic_barfull4" , L"tactic_barfull5"  };
};
struct HUDBars
{
	BarKeys leader{ L"leader_hpbarfront", L"leader_redbarfront" };
	array<BarKeys, 2> party = {
		BarKeys{ L"party_hpbarfront1", L"party_redbarfront1" },
		BarKeys{ L"party_hpbarfront2", L"party_redbarfront2" }
	};
};
struct DamageGlowState
{
	bool  active   = false;
	float t        = 0.f;
	float duration = 5.f;   // ¸î ÃÊ µ¿¾È ±ôºýÀÏÁö
	float freq     = 2.f;   // ÃÊ´ç ±ôºýÀÓ È½¼ö
	float minA     = 0.2f;  // ÃÖ¼Ò ¾ËÆÄ
	float maxA     = 0.9f;  // ÃÖ´ë ¾ËÆÄ
};
struct HUDTextKeys
{
	wstring hpBase = L"battle_hp_text_base";
	wstring hpLeader = L"battle_hp_text_leader";
	array<wstring, 2> hpParty =
	{
		L"battle_hp_text_party0",
		L"battle_hp_text_party1"
	};
	wstring apLeader = L"battle_ap_text_leader";
	wstring tacticLv = L"battle_tactic_lv_text";
};
struct HUDConfig
{
	HUDLayout   layout;
	HUDScales   scales;
	HUDKeys     keys;
	HUDBars     bars;
	TacticKeys  tactic;
	HUDTextKeys texts;
};
struct HPBarState   { float ratio = 0.f; };
struct StunBarState { float ratio = 0.f; };
struct PinchState
{
	bool  active   = false;
	float t        = 0.f;
	float freq     = 1.8f;
	float minScale = 1.f;
	float maxScale = 1.3f;
};
struct TacticBlink
{
	bool  active = false;
	float t      = 0.f;
	float freq   = 2.f;
	float minA   = 0.35f;
	float maxA   = 1.f;
};
struct TacticRuntime
{
	int         level     = 1;
	int         pips      = 0;
	float       revealA   = 1.f;
	int         revealIdx = -1;
	TacticBlink blink;

	bool  lvBannerVisible   = false;
	bool  lvBannerFadingOut = false;
	float lvBannerTimer     = 0.f;
};
struct DigitSlot
{
	float anchorX = 0.f;
	float anchorY = 0.f;
	float gapX = 18.f;
	float scaleX = 1.f;
	float scaleY = 1.f;
	vector<wstring> keys;
};
struct HUDRuntime
{
	EntityID               leader = invalidEntity;
	array<EntityID, 2>     party{ invalidEntity, invalidEntity };
	wstring                appliedLeaderTex;
	array<wstring, 2>      appliedPartyTex;
	HPBarState             leaderHP;
	array<HPBarState, 2>   partyHP;
	StunBarState           leaderStun;          
	array<StunBarState, 2> partyStun;    
	array<PinchState, 2>   pinch;
	TacticRuntime          tactic;
	DamageGlowState        glowLeft;
	DamageGlowState        glowRight;

	array<float, 2>        prevHpRatioForGlow{ 1.f, 1.f };

	DigitSlot              hpLeaderDigits;
	array<DigitSlot, 2>    hpPartyDigits;
	DigitSlot              apLeaderDigits;
	DigitSlot              tacticLvDigits;
};


NS_END