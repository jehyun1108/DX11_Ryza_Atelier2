#pragma once

NS_BEGIN(Engine)

struct BattleBoardLayout
{
	_float2 centerLocal{ 1050.f, -500.f }; 
	float   boardRadius = 120.f;        
	float   maxWorldRadius = 1000.f;    
};
struct BattleBoardConfig
{
	wstring boardKey    = L"battle_minimap";
	wstring iconBaseKey = L"battle_board_icon_base";
};
struct BattleBoardSlot
{
	EntityID   entity{};
	BattleTeam team{};
	int        idx{};
};

NS_END