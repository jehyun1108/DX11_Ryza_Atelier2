#pragma once

NS_BEGIN(Engine)
enum class DressingFocus
{
	CharacterTabs, CostumeList
};
enum class DressingRoomState
{
	Hidden, Entering, Idle, Exiting
};
struct CostumeTextureSwap
{
	wstring     baseTexKey;          
	wstring     variantTexKey;       
};
struct CostumeDef
{
	int     idx = 0;
	wstring displayName;              
	vector<CostumeTextureSwap> swaps;
	wstring equipSfxKey;
};
struct CharacterDressingData
{
	EntityID characterEntity = 0u;     
	wstring  nameText;                 

	wstring  tabNameKey;               
	wstring  tabEquippedKey;          

	vector<CostumeDef> costumes;     

	int selectedIdx = 0;
	int equippedIdx = 0;

	wstring dressingIdleClip;
	wstring dressingChangeClip;
	Handle  animHandle;
};
// UI ÂÊ
struct CostumeRowUI
{
	wstring textUIKey;
	wstring circleKey;
	wstring checkKey;
};
struct CostumeListUI 
{
	wstring bgKey;
	wstring highlightKey;

	wstring rowTextBaseKey;
	wstring rowCircleBaseKey;
	wstring rowCheckBaseKey;

	wstring         dividerBaseKey;         
	float           dividerOffsetY = 40.f;   
	vector<wstring> dividerKeys;   

	float                rowOffsetY = 40.f;
	vector<CostumeRowUI> rows;
};
struct DressingRoomUIConfig
{
	UIContext       context = UIContext::Field;

	wstring         bgKey;
	wstring         tabPaperKey;

	wstring         tabHighlightKey;   
	wstring         tabHighlightBaseKey; 
	float           tabRowOffsetY = 50.f;

	wstring         tabDividerBaseKey;     
	float           tabDividerOffsetY = 40.f;
	vector<wstring> tabDividerKeys;    

	wstring         tabEquipBarBaseKey;   
	float           tabEquipBarOffsetY = 40.f;
	vector<wstring> tabEquipBarKeys;

	CostumeListUI listUI;
};

NS_END