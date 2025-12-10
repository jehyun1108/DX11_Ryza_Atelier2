#include "Enginepch.h"
#include "FieldMinimapPresenter.h"
#include "UIMinimapSystem.h"

void FieldMinimapPresenter::OnBoot()
{
	uiRegistry = &registry.Get<UIRegistry>();
	uiAnimSys  = &registry.Get<UIAnimSystem>();
	minimapSys = &registry.Get<UIMinimapSystem>();
	tfSys      = &registry.Get<TransformSystem>();
	input      = &registry.Get<InputService>();
	layerSys   = &registry.Get<LayerSystem>();
	uiSys      = &registry.Get<UISystem>();
}

void FieldMinimapPresenter::Enter()
{
	minimapSys->SetMode(MinimapMode::Field);

	minimapSys->SetFieldTarget(playerTf);
	minimapSys->SetBattleTarget(_float3{ 0.f, 0.f, 0.f }, worldRadius);

	uiRegistry->Ensure(minimapFrameKey);
	uiRegistry->Ensure(minimapMapKey);
	uiRegistry->Ensure(minimapPlayerKey);
	uiRegistry->Ensure(minimapNorthKey);

	uiRegistry->SetEnabled(minimapFrameKey,  true);
	uiRegistry->SetEnabled(minimapMapKey,    true);
	uiRegistry->SetEnabled(minimapPlayerKey, true);
	uiRegistry->SetEnabled(minimapNorthKey,  true);

	uiAnimSys->SetScale(minimapPlayerKey, 1.f, 1.f);
	uiAnimSys->SetOpacity(minimapPlayerKey, 1.f);
}

void FieldMinimapPresenter::Tick(float dt)
{
	UpdateCam(dt);
	UpdatePlayerCursor();
	UpdateEnemyIcons();
	UpdateMapScroll();
}

void FieldMinimapPresenter::Exit()
{
	uiRegistry->SetEnabled(minimapFrameKey, false);
	uiRegistry->SetEnabled(minimapMapKey, false);
	uiRegistry->SetEnabled(minimapPlayerKey, false);
	uiRegistry->SetEnabled(minimapNorthKey, false);

	minimapSys->SetMode(MinimapMode::None);
}

void FieldMinimapPresenter::SetPlayer(EntityID _playerID, Handle _playerTf)
{
	playerID = _playerID;
	playerTf = _playerTf;
	minimapSys->SetFieldTarget(playerTf);
}

void FieldMinimapPresenter::SetFieldCam(Handle camTf, Handle cam)
{
	fieldCamTf = camTf;
	fieldCam   = cam;
	minimapSys->SetFieldCam(cam);
}

MinimapScreenRect FieldMinimapPresenter::GetMinimapScreenRect(const wstring& mapKey)
{
	MinimapScreenRect r{};

	const auto& instances = uiRegistry->GetInstances();
	auto it = instances.find(mapKey);
	assert(it != instances.end());
	const UIInstance& inst = it->second;
	const UIArchetypeSpec& spec = *inst.spec;

	const auto& vp = GAME.GetViewport();
	const float screenW = static_cast<float>(vp.Width);
	const float screenH = static_cast<float>(vp.Height);

	const wstring texKey = inst.overrideKey ? *inst.overrideKey : spec.texKey;
	auto sizeWH = uiRegistry->GetOrCacheTexSize(texKey);
	float srcW = sizeWH.first;
	float srcH = sizeWH.second;

	float drawW = srcW;
	float drawH = srcH;
	switch (spec.sizeMode)
	{
	case UISizeMode::Original: break;
	case UISizeMode::Fixed:
		drawW = spec.fixedWidth;
		drawH = spec.fixedHeight;
		break;
	case UISizeMode::Ratio:
		drawW = srcW * spec.ratioX;
		drawH = srcH * spec.ratioY;
		break;
	}

	const float scaledW = drawW * inst.animScaleX;
	const float scaledH = drawH * inst.animScaleY;

	auto [axN, ayN] = uiSys->ToNorm(spec.anchor);
	const float anchorX = axN * screenW;
	const float anchorY = ayN * screenH;

	const float centerX = anchorX + inst.localX + inst.animOffsetX;
	const float centerY = anchorY + inst.localY + inst.animOffsetY;

	r.centerX = centerX;
	r.centerY = centerY;
	r.radiusPx = 0.5f * min(scaledW, scaledH);
	r.anchorX = anchorX;
	r.anchorY = anchorY;
	return r;
}

void FieldMinimapPresenter::UpdateCam(float dt)
{
	if (!playerTf.IsValid() || !fieldCamTf.IsValid()) return;

	TransformData* camTf = tfSys->Get(fieldCamTf);
	const TransformData* playerTfData = tfSys->Get(playerTf);

	_float3 camPos = playerTfData->pos;
	camPos.y += camHeight;

	tfSys->SetPos(fieldCamTf, camPos);
	tfSys->SetEuler(fieldCamTf, camPitchDeg, 0.f, 0.f);
}

void FieldMinimapPresenter::UpdatePlayerCursor()
{
	if (!playerTf.IsValid()) return;

	_float2 fwdXZ = tfSys->GetForwardXZ(playerTf); 

	float mapX = fwdXZ.x;
	float mapY = fwdXZ.y; 

	//float lenSq = mapX * mapX + mapY * mapY;
	//if (lenSq < 1e-8f) return;

	float yawRad = atan2f(mapX, mapY);
	float yawDeg = XMConvertToDegrees(yawRad);

	uiAnimSys->SetRotDeg(minimapPlayerKey, yawDeg);
}

void FieldMinimapPresenter::UpdateEnemyIcons()
{ 
	auto r = GetMinimapScreenRect(minimapCentralKey);
	enemyIconCount = 0;

	const TransformData* playerTfData = tfSys->Get(playerTf);
	const _float3 playerPos = playerTfData->pos;

	float uPlayer, vPlayer;
	if (!minimapSys->WorldToMiniUV_Global(playerPos, uPlayer, vPlayer)) return;

	constexpr _uint monsterMask = LayerUtil::LayerBit(LAYER::MONSTER);

	layerSys->ForEachByMask(monsterMask, [&](EntityID owner, Handle layerH, const LayerData& layer)
		{
			const TransformData* tf = tfSys->Get(layer.transform);
			const _float3 pos = tf->pos;

			float u, v;
			if (!minimapSys->WorldToMiniUV_Global(pos, u, v))
				return;

			float du = u - uPlayer;   
			float dv = vPlayer - v;

			float nx = du * 2.0f;
			float ny = dv * 2.0f;

			float r2 = nx * nx + ny * ny;
			if (r2 > 1.0f) return;

			float px = nx * r.radiusPx;
			float py = -ny * r.radiusPx; 

			float iconScreenX = r.centerX + px;
			float iconScreenY = r.centerY + py;

			wstring key = L"field_minimap_enemy_" + to_wstring(enemyIconCount++);

			UIInstance& inst = uiRegistry->EnsureClone(enemyIconRed, key, owner);
			inst.selfEnabled = true;

			inst.localX = iconScreenX - r.anchorX - inst.animOffsetX;
			inst.localY = iconScreenY - r.anchorY - inst.animOffsetY;
		});

	for (int i = enemyIconCount; ; ++i)
	{
		wstring key = L"field_minimap_enemy_" + to_wstring(i);
		const auto& instances = uiRegistry->GetInstances();
		if (!instances.count(key)) break;
		uiRegistry->SetEnabled(key, false);
	}
}

void FieldMinimapPresenter::UpdateMapScroll()
{
	const TransformData* tf = tfSys->Get(playerTf);
	const _float3 pos = tf->pos;

	float u, v;
	bool inside = minimapSys->WorldToMiniUV_Global(pos, u, v);

	float offsetU = 0.5f - u;
	float offsetV = 0.5f - v;

	uiAnimSys->SetFill(minimapCentralKey, offsetU, offsetV);
}