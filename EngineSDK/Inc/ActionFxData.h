#pragma once

NS_BEGIN(Engine)

using ActionFxKey = _uint;

struct ActionTrailClipFx
{
	AnimKey clipKey;
	wstring effectKey;
	float   startNorm;
	float   endNorm;
	bool    attachToWeapon = true;
};
struct ActionHitFx
{
	wstring effectKey;
	bool    attachToTarget = true;
};
struct ActionCastFx
{
	wstring effectKey;
	float   startNorm = 0.f;
};
struct ActionFxSet
{
	vector<ActionTrailClipFx> trails;

	bool         hasCast = false;
	ActionCastFx cast;

	bool        hasHitFx = false;
	ActionHitFx hitFx;
};


NS_END