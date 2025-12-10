#include "Enginepch.h"
#include "DressingRoomPresenter.h"
#include "SoundSystem.h"

void DressingRoomPresenter::OnBoot()
{
	uiSys       = &registry.Get<UISystem>();
	uiAnim      = &registry.Get<UIAnimSystem>();
	uiReg       = &registry.Get<UIRegistry>();
	dataSys     = &registry.Get<CharacterDataSystem>();
	input       = &registry.Get<InputService>();
	tfSys       = &registry.Get<TransformSystem>();
	modelSys    = &registry.Get<ModelSystem>();
	animator    = &registry.Get<AnimatorSystem>();
	assets      = &registry.Get<AssetSystem>();
	animDataSys = &registry.Get<AnimDataSystem>();
	soundSys    = &registry.Get<SoundSystem>();

	uiConfig.context     = UIContext::Field;
	uiConfig.bgKey       = L"dressing_bg";
	uiConfig.tabPaperKey = L"dressing_paper_tab";

	uiConfig.tabHighlightBaseKey = L"dressing_tab_highlight_base"; // tex: dressing_select_highlight
	uiConfig.tabHighlightKey     = L"dressing_tab_highlight";
	uiConfig.tabRowOffsetY       = 50.f;

	uiConfig.tabDividerBaseKey = L"dressing_divider_tab_base";
	uiConfig.tabDividerOffsetY = 50.f;

	uiConfig.tabEquipBarBaseKey = L"dressing_tab_equipbar_base";  // tex: character_equip_barback
	uiConfig.tabEquipBarOffsetY = 50.f;

	CostumeListUI list{};
	list.bgKey            = L"dressing_paper_list";
	list.highlightKey     = L"dressing_select_highlight";

	list.rowOffsetY       = 50.f;
	list.rowTextBaseKey   = L"dressing_row_text_base";
	list.rowCircleBaseKey = L"dressing_row_circle_base";
	list.rowCheckBaseKey  = L"dressing_row_check_base";

	list.dividerBaseKey = L"dressing_divider_list_base";
	list.dividerOffsetY = 50.f;

	const int visibleRowCount = 6;
	for (int i = 0; i < visibleRowCount; ++i)
	{
		CostumeRowUI row{};
		row.textUIKey = L"dressing_row_text_" + to_wstring(i);
		row.circleKey = L"dressing_row_circle_" + to_wstring(i);
		row.checkKey  = L"dressing_row_check_" + to_wstring(i);
		list.rows.push_back(row);
	}

	const int tabSlotCount = 3; 
	for (int i = 0; i < tabSlotCount; ++i)
	{
		uiConfig.tabDividerKeys.push_back(L"dressing_tab_divider_" + to_wstring(i));
		uiConfig.tabEquipBarKeys.push_back(L"dressing_tab_equipbar_" + to_wstring(i));
	}

	const int listDividerCount = 5;
	for (int i = 0; i < listDividerCount; ++i)
		list.dividerKeys.push_back(L"dressing_list_divider_" + to_wstring(i));

	uiConfig.listUI = list;
}

void DressingRoomPresenter::Enter()
{
	state = DressingRoomState::Entering;
	soundSys->Play(L"enter_dressing", 0.3f);

	SetUpUIInstances();
	RefreshTabs();
	RefreshCostumeList();

	uiReg->SetEnabled(uiConfig.bgKey, true);
	uiReg->SetEnabled(uiConfig.tabPaperKey, true);
	//uiReg->SetEnabled(uiConfig.listUI.bgKey, true);
	uiReg->SetEnabled(L"dressing_char_rt", true);

	for (const wstring& key : uiConfig.tabDividerKeys)
		uiReg->SetEnabled(key, true);

	for (const wstring& key : uiConfig.tabEquipBarKeys)
		uiReg->SetEnabled(key, true);

	for (const wstring& key : uiConfig.listUI.dividerKeys)
		uiReg->SetEnabled(key, true);

	SetCostumePanelVisible(false);

	for (auto& ch : characters)
		animator->CrossFade(ch.animHandle, 0, 1, ch.dressingIdleClip, 0.25f, ANIMTYPE::LOOP);

	prevOrbitAngle = dressingOrbitAngle;
	orbitAccumulatedRad = 0.f;
	ryzaZoomSfxPlayed = false;

	tfSys->SetForward(tfSys->Get(dataSys->GetEntityID(CharacterID::Ryza)), {0.f, 0.f, -1.f});

	state = DressingRoomState::Idle;
}

void DressingRoomPresenter::Tick(float dt)
{
	if (state != DressingRoomState::Idle) return;

	switch (focus)
	{
	case DressingFocus::CharacterTabs:
		HandleTabInput();
		break;
	case DressingFocus::CostumeList:
		HandleCostumeInput();
		break;
	}

	CharacterDressingData& ch = characters[activeCharIdx];
	CharacterID chId = dataSys->GetCharacterID(ch.characterEntity);
	bool isRyza = (chId == CharacterID::Ryza);

	const float rotSpeedDeg = 90.f;
	const float rotSpeedRad = XMConvertToRadians(rotSpeedDeg);

	float oldAngle = dressingOrbitAngle;

	if (input->KeyPressing(KEY::RBUTTON))
		dressingOrbitAngle += rotSpeedRad * dt;
	if (input->KeyPressing(KEY::LBUTTON))
		dressingOrbitAngle -= rotSpeedRad * dt;

	const float twoPi = XM_PI * 2.f;
	if (dressingOrbitAngle > twoPi)
		dressingOrbitAngle -= twoPi;
	else if (dressingOrbitAngle < 0.f)
		dressingOrbitAngle += twoPi;

	float newAngle = dressingOrbitAngle;
	float delta = newAngle - oldAngle;

	if (delta > XM_PI)      delta -= twoPi;
	else if (delta < -XM_PI) delta += twoPi;

	orbitAccumulatedRad += fabsf(delta);

	if (!isRyza)
		orbitAccumulatedRad = 0.f;
	else
	{
		if (orbitAccumulatedRad >= twoPi)
		{
			soundSys->Play(L"ryza_53");
			orbitAccumulatedRad = 0.f;
		}
	}

	prevOrbitAngle = dressingOrbitAngle;

	// ----------------- ÁÜ Ã³¸® + ±ÙÁ¢ SFX -----------------
	const float zoomMin = 1.f;
	const float zoomMax = 4.f;
	const float zoomStep = 0.3f;

	if (input->KeyDown(KEY::WHEEL_DOWN))
		targetDressingZoom -= zoomStep;
	if (input->KeyDown(KEY::WHEEL_UP))
		targetDressingZoom += zoomStep;

	targetDressingZoom = clamp(targetDressingZoom, zoomMin, zoomMax);
	const float zoomSmooth = 10.f;
	float prevZoom = dressingZoom;

	dressingZoom += (targetDressingZoom - dressingZoom) * zoomSmooth * dt;
	dressingZoom = clamp(dressingZoom, zoomMin, zoomMax);

	if (isRyza)
	{
		const float threshold = 1.1f;
		if (!ryzaZoomSfxPlayed && prevZoom > threshold && dressingZoom <= threshold)
		{
			soundSys->Play(L"ryza_51");
			ryzaZoomSfxPlayed = true;
		}
		else if (ryzaZoomSfxPlayed && dressingZoom > threshold + 0.2f)
			ryzaZoomSfxPlayed = false;
	}
	else
		ryzaZoomSfxPlayed = false;

	const float panSpeed = 80.f;
	const float panMax = 120.f;


	if (input->KeyPressing(KEY::D))
		dressingPanRight -= panSpeed * dt;
	if (input->KeyPressing(KEY::A))
		dressingPanRight += panSpeed * dt;

	if (input->KeyPressing(KEY::S))
		dressingPanUp += panSpeed * dt;
	if (input->KeyPressing(KEY::W))
		dressingPanUp -= panSpeed * dt;

	dressingPanRight = clamp(dressingPanRight, -panMax, panMax);
	dressingPanUp = clamp(dressingPanUp, -panMax, panMax);

	for (auto& c : characters)
	{
		if (!c.animHandle.IsValid())
			continue;
		if (c.dressingChangeClip.empty() || c.dressingIdleClip.empty())
			continue;

		if (animator->IsPlayingClip(c.animHandle, 0, c.dressingChangeClip))
		{
			float rem = animator->GetRemainingNormalized(c.animHandle, 0);
			if (rem <= 0.01f)
				animator->Play(c.animHandle, 0, c.dressingIdleClip, ANIMTYPE::LOOP);
		}
	}
}

void DressingRoomPresenter::Exit()
{
	if (state == DressingRoomState::Hidden)
		return;
	soundSys->Play(L"exit_dressing", 0.3f);
	state = DressingRoomState::Hidden;

	uiReg->SetEnabled(uiConfig.bgKey, false);
	uiReg->SetEnabled(uiConfig.tabPaperKey, false);
	uiReg->SetEnabled(uiConfig.listUI.bgKey, false);
	uiReg->SetEnabled(uiConfig.listUI.highlightKey, false);
	uiReg->SetEnabled(uiConfig.tabHighlightKey, false);
	uiReg->SetEnabled(L"dressing_char_rt", false);

	for (const CostumeRowUI& row : uiConfig.listUI.rows)
	{
		uiReg->SetEnabled(row.textUIKey, false);
		uiReg->SetEnabled(row.circleKey, false);
		uiReg->SetEnabled(row.checkKey, false);
	}
	for (const CharacterDressingData& data : characters)
	{
		uiReg->SetEnabled(data.tabNameKey, false);
		uiReg->SetEnabled(data.tabEquippedKey, false);
	}
	for (const wstring& key : uiConfig.tabDividerKeys)
		uiReg->SetEnabled(key, false);

	for (const wstring& key : uiConfig.tabEquipBarKeys)
		uiReg->SetEnabled(key, false);

	for (const wstring& key : uiConfig.listUI.dividerKeys)
		uiReg->SetEnabled(key, false);
}

void DressingRoomPresenter::BuildDressingCamera(CameraProxy& outCam)
{
	const CharacterDressingData& ch = characters[activeCharIdx];
	EntityID entity = ch.characterEntity;

	Handle charTf{};
	bool   found = false;

	modelSys->ForEachAliveEx(
		[&](Handle h, EntityID owner, const ModelData& model)
		{
			if (found) return;
			if (owner != entity) return;
			if (!model.enabled || !model.model) return;

			charTf = model.transform;
			found = true;
		});

	_float3 center = tfSys->GetPos(charTf);
	float   s = 100.f;
	center.y += 1.0f * s; 

	float radius = dressingZoom * s;

	_float3 camPos{};
	camPos.x = center.x + sinf(dressingOrbitAngle) * radius;
	camPos.y = center.y + 0.3f * s;
	camPos.z = center.z + cosf(dressingOrbitAngle) * radius;

	_vec eye = XMVectorSet(camPos.x, camPos.y, camPos.z, 1.f);
	_vec target = XMVectorSet(center.x, center.y, center.z, 1.f);
	_vec up = XMVectorSet(0.f, 1.f, 0.f, 0.f);

	_vec forward = XMVector3Normalize(target - eye);
	_vec right = XMVector3Normalize(XMVector3Cross(up, forward));
	_vec pan = right * dressingPanRight + up * dressingPanUp;

	eye = eye + pan;
	target = target + pan;
	_mat view = XMMatrixLookAtLH(eye, target, up);

	float aspect = 1.0f;
	constexpr float fovY = XMConvertToRadians(35.f);
	float nearZ = 1.0f;
	float farZ = 500.f;

	_mat proj = XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ);
	_mat viewProj = view * proj;
	_mat invView = XMMatrixInverse(nullptr, view);
	_mat invProj = XMMatrixInverse(nullptr, proj);
	_mat invViewProj = XMMatrixInverse(nullptr, viewProj);

	XMStoreFloat4x4(&outCam.view, view);
	XMStoreFloat4x4(&outCam.proj, proj);
	XMStoreFloat4x4(&outCam.viewProj, viewProj);
	XMStoreFloat4x4(&outCam.invView, invView);
	XMStoreFloat4x4(&outCam.invProj, invProj);
	XMStoreFloat4x4(&outCam.invViewProj, invViewProj);

	outCam.zNear  = nearZ;
	outCam.zFar   = farZ;
	outCam.fovY   = fovY;
	outCam.aspect = aspect;

	_float4 camPos4{};
	XMStoreFloat4(reinterpret_cast<_float4*>(&camPos4), eye);
	outCam.camPos = camPos4;

	XMVECTOR fwd = XMVector3Normalize(target - eye);
	_float4 forward4{};
	XMStoreFloat4(reinterpret_cast<_float4*>(&forward4), fwd);
	outCam.camForward = forward4;
}

void DressingRoomPresenter::BuildDressingDrawItems(vector<DrawItem>& outItems)
{
	outItems.clear();
	const CharacterDressingData& ch = characters[activeCharIdx];
	EntityID entity = ch.characterEntity;

	modelSys->ForEachAliveEx(
		[&](Handle handle, EntityID owner, const ModelData& model)
		{
			if (owner != entity)
				return;

			if (!model.enabled || !model.model)
				return;

			const _float4x4* pWorld = tfSys->GetWorld(model.transform);
			assert(pWorld);

			for (const auto& part : model.model->GetParts())
			{
				if (!part.mesh || !part.material)
					continue;
				if (part.mesh->GetUsage() == MESHTYPE::Driver)
					continue;

				RenderProxy proxy{};
				proxy.owner = owner;
				proxy.mesh = part.mesh;
				proxy.material = part.material;
				proxy.world = *pWorld;
				proxy.isSkinned = (part.mesh->GetLayoutID() == VertexLayoutID::PNUTanSkin);
				proxy.layerMask = 0xFFFFFFFF;

				if (proxy.isSkinned)
				{
					const vector<_float4x4>* finalMatrices = nullptr;

					if (model.animator.IsValid())
						finalMatrices = animator->GetFinalMatrices(model.animator);

					if (!finalMatrices || finalMatrices->empty())
						finalMatrices = &model.model->GetBindPoseMatrices();

					proxy.boneMatrices = BoneMatrices{ finalMatrices->data(),static_cast<_uint>(finalMatrices->size()) };
					proxy.skeleton = model.model->GetSkeleton();
				}

				DrawItem item{};
				item.proxy = move(proxy);
				item.layerMask = item.proxy.layerMask;

				outItems.push_back(std::move(item));
			}
		});
}

void DressingRoomPresenter::SetUpUIInstances()
{
	uiReg->Ensure(uiConfig.bgKey);
	uiReg->Ensure(uiConfig.tabPaperKey);

	CostumeListUI& list = uiConfig.listUI;
	uiReg->Ensure(list.bgKey);

	UIInstance& highlightBase = uiReg->Ensure(list.highlightKey);
	UIInstance& textBase      = uiReg->Ensure(list.rowTextBaseKey);
	UIInstance& circleBase    = uiReg->Ensure(list.rowCircleBaseKey);
	UIInstance& checkBase     = uiReg->Ensure(list.rowCheckBaseKey);

	if (!highlightOffsetInit)
	{
		highlightOffsetX = highlightBase.localX - textBase.localX;
		highlightOffsetY = highlightBase.localY - textBase.localY;
		highlightOffsetX += 100.f;
		highlightOffsetInit = true;
	}

	textBase.selfEnabled = false;
	circleBase.selfEnabled = false;
	checkBase.selfEnabled = false;
	highlightBase.selfEnabled = false;
	uiReg->SetEnabled(list.highlightKey, false);

	const float baseTextX = textBase.localX;
	const float baseTextY = textBase.localY;
	const float baseCircleX = circleBase.localX;
	const float baseCircleY = circleBase.localY;
	const float baseCheckX = checkBase.localX;
	const float baseCheckY = checkBase.localY;

	const int rowCount = static_cast<int>(list.rows.size());
	for (int i = 0; i < rowCount; ++i)
	{
		CostumeRowUI& row = list.rows[i];
		const float offsetY = list.rowOffsetY * static_cast<float>(i);
		{
			UIInstance& inst = uiReg->EnsureClone(list.rowTextBaseKey, row.textUIKey);
			inst.selfEnabled = false;
			inst.localX = baseTextX;
			inst.localY = baseTextY + offsetY;
		}
		{
			UIInstance& inst = uiReg->EnsureClone(list.rowCircleBaseKey, row.circleKey);
			inst.selfEnabled = false;
			inst.localX = baseCircleX;
			inst.localY = baseCircleY + offsetY;
		}
		{
			UIInstance& inst = uiReg->EnsureClone(list.rowCheckBaseKey, row.checkKey);
			inst.selfEnabled = false;
			inst.localX = baseCheckX;
			inst.localY = baseCheckY + offsetY;
		}
	}
	for (const CharacterDressingData& data : characters)
	{
		uiReg->Ensure(data.tabNameKey);
		uiReg->Ensure(data.tabEquippedKey);
	}

	UIInstance& tabHighlightBase = uiReg->Ensure(uiConfig.tabHighlightBaseKey);
	CharacterDressingData& firstChar = characters[0];

	if (!tabHighlightOffsetInit)
	{
		float nameX = 0.f, nameY = 0.f;
		uiReg->GetLocalPos(firstChar.tabNameKey, nameX, nameY);

		tabHighlightOffsetX = tabHighlightBase.localX - nameX;
		tabHighlightOffsetY = tabHighlightBase.localY - nameY;
		tabHighlightOffsetInit = true;
	}

	UIInstance& tabHighlight = uiReg->EnsureClone(uiConfig.tabHighlightBaseKey,
		uiConfig.tabHighlightKey);
	tabHighlight.selfEnabled = false;
	uiReg->SetEnabled(uiConfig.tabHighlightKey, false);

	// ===== ÅÇ µð¹ÙÀÌ´õ =====
	UIInstance& tabDivBase = uiReg->Ensure(uiConfig.tabDividerBaseKey);
	tabDivBase.selfEnabled = false;

	const float tabBaseX = tabDivBase.localX;
	const float tabBaseY = tabDivBase.localY;

	const int tabDivCount = static_cast<int>(uiConfig.tabDividerKeys.size());
	for (int i = 0; i < tabDivCount; ++i)
	{
		const wstring& key = uiConfig.tabDividerKeys[i];

		UIInstance& inst = uiReg->EnsureClone(uiConfig.tabDividerBaseKey, key);
		inst.selfEnabled = false;
		inst.localX = tabBaseX;
		inst.localY = tabBaseY + uiConfig.tabDividerOffsetY * static_cast<float>(i);
		uiReg->SetEnabled(key, false);
	}

	UIInstance& equipBase = uiReg->Ensure(uiConfig.tabEquipBarBaseKey);
	equipBase.selfEnabled = false;

	const float equipBaseX = equipBase.localX;
	const float equipBaseY = equipBase.localY;

	const int equipCount = static_cast<int>(uiConfig.tabEquipBarKeys.size());
	for (int i = 0; i < equipCount; ++i)
	{
		const wstring& key = uiConfig.tabEquipBarKeys[i];

		UIInstance& inst = uiReg->EnsureClone(uiConfig.tabEquipBarBaseKey, key);
		inst.selfEnabled = false;
		inst.localX = equipBaseX;
		inst.localY = equipBaseY + uiConfig.tabEquipBarOffsetY * static_cast<float>(i);
		uiReg->SetEnabled(key, false);
	}

	UIInstance& listDivBase = uiReg->Ensure(list.dividerBaseKey);
	listDivBase.selfEnabled = false;

	const float listBaseX = listDivBase.localX;
	const float listBaseY = listDivBase.localY;

	const int listDivCount = static_cast<int>(list.dividerKeys.size());
	for (int i = 0; i < listDivCount; ++i)
	{
		const wstring& key = list.dividerKeys[i];

		UIInstance& inst = uiReg->EnsureClone(list.dividerBaseKey, key);
		inst.selfEnabled = false;
		inst.localX = listBaseX;
		inst.localY = listBaseY + list.dividerOffsetY * static_cast<float>(i);
		uiReg->SetEnabled(key, false);
	}
}

void DressingRoomPresenter::RefreshTabs()
{
	const int count = static_cast<int>(characters.size());

	for (int i = 0; i < count; ++i)
	{
		CharacterDressingData& data = characters[i];
		const CostumeDef& equipped = data.costumes[data.equippedIdx];

		uiReg->SetEnabled(data.tabNameKey, true);
		uiReg->SetEnabled(data.tabEquippedKey, true);
		uiReg->SetText(data.tabNameKey, data.nameText);
		uiReg->SetText(data.tabEquippedKey, equipped.displayName);
	}

	const int barCount = static_cast<int>(uiConfig.tabEquipBarKeys.size());
	for (int i = 0; i < barCount; ++i)
	{
		const bool enable = (i < count);
		uiReg->SetEnabled(uiConfig.tabEquipBarKeys[i], enable);
	}

	RefreshTabHighlight();
}

void DressingRoomPresenter::RefreshTabHighlight()
{
	CharacterDressingData& ch = characters[activeCharIdx];

	float x = 0.f, y = 0.f;
	uiReg->GetLocalPos(ch.tabNameKey, x, y);

	const float hx = x + tabHighlightOffsetX;
	const float hy = y + tabHighlightOffsetY;

	uiReg->SetLocalPos(uiConfig.tabHighlightKey, hx, hy);
	uiReg->SetEnabled(uiConfig.tabHighlightKey, true);
}

void DressingRoomPresenter::RefreshCostumeList()
{
	CharacterDressingData& ch = characters[activeCharIdx];
	const vector<CostumeDef>& costumes = ch.costumes;

	const CostumeListUI& list = uiConfig.listUI;
	const int rowCount = static_cast<int>(list.rows.size());
	const int costumeCount = static_cast<int>(costumes.size());

	bool highlightUsed = false;

	for (int row = 0; row < rowCount; ++row)
	{
		const CostumeRowUI& rowUI = list.rows[row];

		if (row < costumeCount)
		{
			const CostumeDef& def = costumes[row];

			uiReg->SetEnabled(rowUI.textUIKey, true);
			uiReg->SetEnabled(rowUI.circleKey, true);
			uiReg->SetEnabled(rowUI.checkKey, true);

			uiReg->SetText(rowUI.textUIKey, def.displayName);

			const bool equipped = (row == ch.equippedIdx);
			uiReg->SetEnabled(rowUI.checkKey, equipped);

			if (row == ch.selectedIdx)
			{
				float x = 0.f, y = 0.f;
				uiReg->GetLocalPos(rowUI.textUIKey, x, y);

				uiReg->SetLocalPos( list.highlightKey, x + highlightOffsetX, y + highlightOffsetY );
				uiReg->SetEnabled(list.highlightKey, true);
				highlightUsed = true;
			}
		}
		else
		{
			uiReg->SetEnabled(rowUI.textUIKey, false);
			uiReg->SetEnabled(rowUI.circleKey, false);
			uiReg->SetEnabled(rowUI.checkKey, false);
		}
	}
	if (!highlightUsed)
		uiReg->SetEnabled(list.highlightKey, false);
}

void DressingRoomPresenter::SetCostumePanelVisible(bool visible)
{
	CostumeListUI& list = uiConfig.listUI;
	uiReg->SetEnabled(list.bgKey, visible);

	if (!visible)
	{
		uiReg->SetEnabled(list.highlightKey, false);

		for (const CostumeRowUI& row : list.rows)
		{
			uiReg->SetEnabled(row.textUIKey, false);
			uiReg->SetEnabled(row.circleKey, false);
			uiReg->SetEnabled(row.checkKey, false);
		}
		for (const wstring& key : list.dividerKeys)
			uiReg->SetEnabled(key, false);
		return;
	}
	RefreshCostumeList();
	for (const wstring& key : list.dividerKeys)
		uiReg->SetEnabled(key, true);
}

void DressingRoomPresenter::HandleTabInput()
{
	const int charCount = static_cast<int>(characters.size());
	if (charCount == 0) return;

	int prev = activeCharIdx;

	if (input->KeyDown(KEY::UP))
	{
		activeCharIdx--;
		if (activeCharIdx < 0)
			activeCharIdx = charCount - 1;
	}
	else if (input->KeyDown(KEY::DOWN))
	{
		activeCharIdx++;
		if (activeCharIdx >= charCount)
			activeCharIdx = 0;
	}

	if (activeCharIdx != prev)
		RefreshTabHighlight();

	if (input->KeyDown(KEY::ENTER))
	{
		CharacterDressingData& ch = characters[activeCharIdx];
		ch.selectedIdx = ch.equippedIdx;
		SetCostumePanelVisible(true);
		focus = DressingFocus::CostumeList;
		soundSys->Play(L"012_chara_select", 0.3f);
	}
}

void DressingRoomPresenter::HandleCostumeInput()
{
	CharacterDressingData& ch = characters[activeCharIdx];
	const int costumeCount = static_cast<int>(ch.costumes.size());
	if (costumeCount == 0) return;

	int prev = ch.selectedIdx;

	if (input->KeyDown(KEY::UP))
	{
		ch.selectedIdx--;
		if (ch.selectedIdx < 0)
			ch.selectedIdx = costumeCount - 1;
	}
	else if (input->KeyDown(KEY::DOWN))
	{
		ch.selectedIdx++;
		if (ch.selectedIdx >= costumeCount)
			ch.selectedIdx = 0;
	}

	if (ch.selectedIdx != prev)
	{
		RefreshCostumeList();
		soundSys->Play(L"013_click", 0.3f);
	}

	if (input->KeyDown(KEY::ENTER))
		EquipSelected();

	if (input->KeyDown(KEY::BACKSPACE))
	{
		soundSys->Play(L"014_cancel", 0.3f);
		SetCostumePanelVisible(false);
		focus = DressingFocus::CharacterTabs;
	}
}

void DressingRoomPresenter::EquipSelected()
{
	CharacterDressingData& ch = characters[activeCharIdx];
	if (ch.costumes.empty())
		return;

	ch.equippedIdx = ch.selectedIdx;
	const CostumeDef& def = ch.costumes[ch.equippedIdx];

	ApplyCostume(ch, ch.costumes[ch.equippedIdx]);

	if (ch.animHandle.IsValid() && !ch.dressingChangeClip.empty())
		animator->Play(ch.animHandle, 0, ch.dressingChangeClip, ANIMTYPE::ONCE);

	if (!def.equipSfxKey.empty())
		soundSys->Play(def.equipSfxKey);

	RefreshTabs();
	RefreshCostumeList();
}

void DressingRoomPresenter::ApplyCostume(const CharacterDressingData& ch, const CostumeDef& def)
{
	EntityID entity = ch.characterEntity;

	auto& shaderCache = assets->GetShaderCache();
	auto& textureCache = assets->GetTextureCache();

	modelSys->ForEachAliveEx(
		[&](Handle handle, EntityID owner, ModelData& model)
		{
			if (owner != entity)
				return;
			if (!model.enabled || !model.model)
				return;

			for (auto& part : model.model->GetParts())
			{
				if (!part.material)
					continue;

				Material* mat = part.material.get();
				MaterialMeta meta = mat->GetMeta();

				bool changed = false;

				for (const CostumeTextureSwap& swap : def.swaps)
				{
					for (_uint slotIdx = 0; slotIdx < NUM_TEXSLOTS; ++slotIdx)
					{
						wstring& curKey = meta.texKey[slotIdx];
						if (curKey.empty())
							continue;
						if (curKey.rfind(swap.baseTexKey, 0) == 0)
						{
							curKey = swap.variantTexKey;
							changed = true;
						}
					}
				}

				if (changed)
				{
					mat->SetMeta(meta);
					mat->Resolve(shaderCache, textureCache);
				}
			}
		});
}

void DressingRoomPresenter::BuildInitData()
{
	characters.clear();

	EntityID ryzaID     = dataSys->GetEntityID(CharacterID::Ryza);
	EntityID patriciaID = dataSys->GetEntityID(CharacterID::Patricia);
	EntityID klaudiaID  = dataSys->GetEntityID(CharacterID::Klaudia);
	// ================== Ryza ==================================
	{
		CharacterDressingData data{};
		data.characterEntity    = ryzaID;
		data.nameText           = L"Ryza";
		data.tabNameKey         = L"dressing_tab_ryza_name";
		data.tabEquippedKey     = L"dressing_tab_ryza_equip";
		data.dressingIdleClip   = animDataSys->GetClipName(CharacterID::Ryza, AnimContext::Field, AnimKey::Idle);
		data.dressingChangeClip = animDataSys->GetClipName(CharacterID::Ryza, AnimContext::Field, AnimKey::Dressing_Change);
		{
			CostumeDef def{};
			def.idx = 0;
			def.displayName = L"Countryside Alchemist";
			CostumeTextureSwap swap{};
			swap.baseTexKey    = L"ryza/0";
			swap.variantTexKey = L"ryza/0";
			def.equipSfxKey = L"ryza_55";
			def.swaps.push_back(swap);
			data.costumes.push_back(def);
		}
		{
			CostumeDef def{};
			def.idx = 1;
			def.displayName = L"Retro";
			CostumeTextureSwap swap{};
			swap.baseTexKey    = L"ryza/0";
			swap.variantTexKey = L"ryza/0_0";
			def.equipSfxKey = L"ryza_55";
			def.swaps.push_back(swap);
			data.costumes.push_back(def);
		}
		{
			CostumeDef def{};
			def.idx = 2;
			def.displayName = L"Danger Zone";
			CostumeTextureSwap swap{};
			swap.baseTexKey    = L"ryza/0";
			swap.variantTexKey = L"ryza/0_1";
			def.equipSfxKey = L"ryza_55";
			def.swaps.push_back(swap);
			data.costumes.push_back(def);
		}
		{
			CostumeDef def{};
			def.idx = 3;
			def.displayName = L"Capital Orange";
			CostumeTextureSwap swap{};
			swap.baseTexKey    = L"ryza/0";
			swap.variantTexKey = L"ryza/0_2";
			def.equipSfxKey = L"ryza_55";
			def.swaps.push_back(swap);
			data.costumes.push_back(def);
		}
		{
			CostumeDef def{};
			def.idx = 4;
			def.displayName = L"Countryside Alchemist Wet";
			CostumeTextureSwap swap{};
			swap.baseTexKey    = L"ryza/0";
			swap.variantTexKey = L"ryza/0_3";
			def.equipSfxKey = L"ryza_55";
			def.swaps.push_back(swap);
			data.costumes.push_back(def);
		}
		data.selectedIdx = 0;
		data.equippedIdx = 0;
		characters.push_back(data);
	}
	// ================== Klaudia ===================================
	{
		CharacterDressingData data{};
		data.characterEntity    = klaudiaID;
		data.nameText           = L"klaudia";
		data.tabNameKey         = L"dressing_tab_klaudia_name";
		data.tabEquippedKey     = L"dressing_tab_klaudia_equip";
		data.dressingIdleClip   = animDataSys->GetClipName(CharacterID::Klaudia, AnimContext::Field, AnimKey::Dressing_Idle);
		data.dressingChangeClip = animDataSys->GetClipName(CharacterID::Klaudia, AnimContext::Field, AnimKey::Dressing_Change);
		{
			CostumeDef def{};
			def.idx = 0;
			def.displayName = L"Lady's Daily Life";
			CostumeTextureSwap swap{};
			swap.baseTexKey    = L"klaudia/0";
			swap.variantTexKey = L"klaudia/0";
			def.equipSfxKey = L"klaudia_39";
			def.swaps.push_back(swap);
			data.costumes.push_back(def);
		}
		{
			CostumeDef def{};
			def.idx = 1;
			def.displayName = L"Jade Dress";
			CostumeTextureSwap swap{};
			swap.baseTexKey    = L"klaudia/0";
			swap.variantTexKey = L"klaudia/0_0";
			def.equipSfxKey = L"klaudia_39";
			def.swaps.push_back(swap);
			data.costumes.push_back(def);
		}
		{
			CostumeDef def{};
			def.idx = 2;
			def.displayName = L"Reisalin Color";
			CostumeTextureSwap swap{};
			swap.baseTexKey    = L"klaudia/0";
			swap.variantTexKey = L"klaudia/0_1";
			def.equipSfxKey = L"klaudia_39";
			def.swaps.push_back(swap);
			data.costumes.push_back(def);
		}
		{
			CostumeDef def{};
			def.idx = 3;
			def.displayName = L"White Bird of the East";
			CostumeTextureSwap swap{};
			swap.baseTexKey    = L"klaudia/0";
			swap.variantTexKey = L"klaudia/0_2";
			def.equipSfxKey = L"klaudia_39";
			def.swaps.push_back(swap);
			data.costumes.push_back(def);
		}
		{
			CostumeDef def{};
			def.idx = 4;
			def.displayName = L"Lady's Daily Life Wet";
			CostumeTextureSwap swap{};
			swap.baseTexKey    = L"klaudia/0";
			swap.variantTexKey = L"klaudia/0_3";
			def.equipSfxKey = L"klaudia_39";
			def.swaps.push_back(swap);
			data.costumes.push_back(def);
		}
		data.selectedIdx = 0;
		data.equippedIdx = 0;
		characters.push_back(data);
	}
	// ========= Patricia ==================
	{
		CharacterDressingData data{};
		data.characterEntity    = patriciaID;
		data.nameText           = L"patricia";
		data.tabNameKey         = L"dressing_tab_patricia_name";
		data.tabEquippedKey     = L"dressing_tab_patricia_equip";
		data.dressingIdleClip   = animDataSys->GetClipName(CharacterID::Patricia, AnimContext::Field, AnimKey::Dressing_Idle);
		data.dressingChangeClip = animDataSys->GetClipName(CharacterID::Patricia, AnimContext::Field, AnimKey::Dressing_Change);
		{
			CostumeDef def{};
			def.idx = 0;
			def.displayName = L"Knightly Lady";
			CostumeTextureSwap swap{};
			swap.baseTexKey    = L"patricia/0";
			swap.variantTexKey = L"patricia/0";
			def.equipSfxKey = L"patricia_37";
			def.swaps.push_back(swap);
			data.costumes.push_back(def);
		}
		{
			CostumeDef def{};
			def.idx = 1;
			def.displayName = L"Urban Sailor";
			CostumeTextureSwap swap{};
			swap.baseTexKey    = L"patricia/0";
			swap.variantTexKey = L"patricia/0_0";
			def.equipSfxKey = L"patricia_37";
			def.swaps.push_back(swap);
			data.costumes.push_back(def);
		}
		{
			CostumeDef def{};
			def.idx = 2;
			def.displayName = L"Evening Ball";
			CostumeTextureSwap swap{};
			swap.baseTexKey    = L"patricia/0";
			swap.variantTexKey = L"patricia/0_1";
			def.equipSfxKey = L"patricia_37";
			def.swaps.push_back(swap);
			data.costumes.push_back(def);
		}
		{
			CostumeDef def{};
			def.idx = 3;
			def.displayName = L"Cool Chocolate Mint";
			CostumeTextureSwap swap{};
			swap.baseTexKey    = L"patricia/0";
			swap.variantTexKey = L"patricia/0_2";
			def.equipSfxKey = L"patricia_37";
			def.swaps.push_back(swap);
			data.costumes.push_back(def);
		}
		{
			CostumeDef def{};
			def.idx = 4;
			def.displayName = L"Knightly Lady Wet";
			CostumeTextureSwap swap{};
			swap.baseTexKey    = L"patricia/0";
			swap.variantTexKey = L"patricia/0_3";
			def.equipSfxKey = L"patricia_37";
			def.swaps.push_back(swap);
			data.costumes.push_back(def);
		}
		data.selectedIdx = 0;
		data.equippedIdx = 0;
		characters.push_back(data);
	}
	activeCharIdx = 0;

	for (auto& ch : characters)
		animator->GetByOwner(ch.characterEntity, &ch.animHandle);
}