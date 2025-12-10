#include "pch.h"
#include "EffectPanel.h"
#include "EffectSerializer.h"

EffectPanel::EffectPanel(string title, SystemRegistry& registry, EntityID* selected) 
	: GuiPanel(move(title), registry, selected)
{
	effectSys = &registry.Get<EffectSystem>();
	camSys    = &registry.Get<CameraSystem>();
	tfSys     = &registry.Get<TransformSystem>();
	input     = &registry.Get<InputService>(); 
    assets    = &registry.Get<AssetSystem>();
    effectSer = &registry.Get<EffectSerializer>();

	state.preview.handle = 0;
	state.preview.dist   = 50.f;
	state.statusMsg.clear();
    state.previewOwner = 2;
}

void EffectPanel::Draw()
{
#ifdef USE_IMGUI
    ImGui::TextUnformatted("Effect Tool");
    ImGui::Separator();
    // 1) 파일 열기 / 저장 / 새 이펙트
    DrawAssetSection();
    ImGui::Separator();
    // 2) 현재 이펙트 정보 표시
    DrawInfoSection();
    ImGui::Separator();
    // 3) 프리뷰 설정 + 재생
    DrawPreviewSettings();
    ImGui::Separator();
    DrawPreviewPlay();
    // 4) 상태 메시지
    if (!state.statusMsg.empty())
    {
        ImGui::Spacing();
        ImGui::TextWrapped("%s", state.statusMsg.c_str());
    }
#endif
}

void EffectPanel::DrawAssetSection()
{
#ifdef USE_IMGUI
    ImGui::TextUnformatted("Effect Asset");

    if (ImGui::Button("New Effect", ImVec2(-1, 0)))
        NewEffect();

    if (ImGui::Button("Open Effect Binary", ImVec2(-1, 0)))
        OpenEffect();

    if (ImGui::Button("Save Effect", ImVec2(-1, 0)))
        SaveEffect();

    if (!state.preview.effectPath.empty())
    {
        ImGui::Text("Current File: %s",
            state.preview.effectPath.filename().string().c_str());
    }
#endif
}

void EffectPanel::DrawInfoSection()
{
#ifdef USE_IMGUI
    if (!state.hasEditing)
    {
        ImGui::TextUnformatted("No effect loaded.");
        return;
    }

    effectSys->RenderArchetypeGui(state.editing);
    state.dirty = true;
#endif
}   

void EffectPanel::DrawPreviewSettings()
{
#ifdef USE_IMGUI
    ImGui::TextUnformatted("Preview Settings");
    ImGui::SliderFloat("Distance", &state.preview.dist, 1.f, 100.f);

    ImGui::Checkbox("Loop Preview", &state.loopPreview);

    if (ImGui::Checkbox("Trail Always On (Preview)", &state.trailAlwaysOnPreview))
        effectSys->SetDebugTrailAlwaysOn(state.trailAlwaysOnPreview);

    ImGui::SeparatorText("Preview Attach");

    int id = static_cast<int>(state.previewOwner);
    if (ImGui::InputInt("Preview EntityID", &id))
    {
        if (id < 0) id = 0;
        state.previewOwner = static_cast<EntityID>(id);
    }
#endif
}

void EffectPanel::DrawPreviewPlay()
{
#ifdef USE_IMGUI
    ImGui::TextUnformatted("Preview Play");

    if (ImGui::Button("Play Preview", ImVec2(-1, 0)))
        PlayPreview();

    if (ImGui::Button("Stop Preview", ImVec2(-1, 0)))
    {
        if (state.preview.handle != 0 && effectSys->IsAlive(state.preview.handle))
        {
            effectSys->Stop(state.preview.handle);
            state.preview.handle = 0;
            state.statusMsg = "Preview stopped";
        }
    }

    if (state.loopPreview && state.preview.handle != 0)
    {
        if (!effectSys->IsAlive(state.preview.handle))
            PlayPreview();
    }
#endif
}

void EffectPanel::NewEffect()
{
    state.editing    = MakeDefaultEffect();
    state.hasEditing = true;
    state.dirty      = true;
    state.preview.effectPath.clear();
    state.preview.effectKey = state.editing.key;
    state.preview.handle = 0;
    effectSys->RegisterArchetype(state.editing);
    state.statusMsg = "New effect created";
}

void EffectPanel::OpenEffect()
{
    constexpr wchar_t effectFilter[] = L"Effect files (*.effect;*.eft;*.dat)\0*.effect;*.eft;*.dat\0All Files (*.*)\0*.*\0";

    auto maybePath = Utility::OpenFileDialog(effectFilter, L"effect;eft;dat");
    if (!maybePath)
    {
        state.statusMsg = "Open canceled";
        return;
    }

    EffectArchetype loaded{};
    if (!effectSer->Load(loaded, *maybePath))
    {
        state.statusMsg = "Effect load failed";
        return;
    }

    assets->EnsureEffectTextures(loaded);

    state.preview.effectPath = *maybePath;
    state.preview.effectKey = loaded.key;
    state.preview.handle = 0;
    state.editing = std::move(loaded);
    state.hasEditing = true;
    state.dirty = false;

    effectSys->RegisterArchetype(state.editing);

    state.statusMsg = "Loaded: " + state.preview.effectPath.filename().string();
}

void EffectPanel::SaveEffect()
{
    if (!state.hasEditing)
    {
        state.statusMsg = "No effect loaded";
        return;
    }

    constexpr wchar_t effectFilter[] =
        L"Effect files (*.effect;*.eft;*.dat)\0*.effect;*.eft;*.dat\0All Files (*.*)\0*.*\0";

    filesystem::path savePath = state.preview.effectPath;

    if (savePath.empty())
    {
        auto maybeOut = Utility::SaveFileDialog(effectFilter, L"effect.effect", L"effect");
        if (!maybeOut)
        {
            state.statusMsg = "Save canceled";
            return;
        }

        savePath = *maybeOut;
        state.preview.effectPath = savePath;

        state.editing.key = savePath.stem().wstring();   // ← 추가
    }

    effectSys->RegisterArchetype(state.editing);

    if (effectSer->Save(state.editing, savePath))
    {
        state.dirty = false;
        state.statusMsg = "Saved: " + savePath.filename().string();
    }
    else
        state.statusMsg = "Save failed";
}

void EffectPanel::PlayPreview()
{
    if (!state.hasEditing)
    {
        state.statusMsg = "No effect loaded to preview";
        return;
    }

    if (state.preview.handle != 0 && effectSys->IsAlive(state.preview.handle))
        effectSys->Stop(state.preview.handle);

    effectSys->RegisterArchetype(state.editing);
    const wstring& key = state.editing.key;

    if (state.previewOwner != 0)
    {
        TransformData* tf = nullptr;
        tf = tfSys->Get(tfSys->Get(state.previewOwner));
        _float3 localOffset{ 0.f, 0.f, 0.f };

        state.preview.handle = effectSys->PlayAttached(key, state.previewOwner, localOffset);
        state.preview.effectKey = key;

        const string keyStr = Utility::ToString(key);
        state.statusMsg = "Play attached preview: " + keyStr + " (EntityID=" + to_string(state.previewOwner) + ")";
        return;
    }

    const CameraData* cam = camSys->Get(camSys->GetMainCamHandle());
    _float3 camPos = { cam->camPos.x, cam->camPos.y, cam->camPos.z };
    _float3 forward{};
    {
        const _float4x4& invView = cam->invView;
        forward.x = invView._31;
        forward.y = invView._32;
        forward.z = invView._33;

        _vec f = XMLoadFloat3(&forward);
        f = XMVector3Normalize(f);
        XMStoreFloat3(&forward, f);
    }

    _float3 spawnPos{};
    spawnPos.x = camPos.x + forward.x * state.preview.dist;
    spawnPos.y = camPos.y + forward.y * state.preview.dist;
    spawnPos.z = camPos.z + forward.z * state.preview.dist;

    state.preview.handle = effectSys->PlayAt(key, spawnPos);
    state.preview.effectKey = key;

    const string keyStr = Utility::ToString(key);
    state.statusMsg = "Play preview at camera: " + keyStr;
}

EffectArchetype EffectPanel::MakeDefaultEffect() const
{
    EffectArchetype effect{};
    effect.key = L"NewEffect";
    effect.duration = 3.f;

    EffectEmitterDesc emitter{};
    emitter.name = L"Emitter0";
    emitter.kind = EffectEmitterKind::Particle;
    emitter.localOffset = _float3(0.f, 0.f, 0.f);
    emitter.spaceMode = EffectSpaceMode::Local;

    emitter.particle.texKey = L"particle_default";

    effectSys->ApplyEmitterPreset(emitter, EmitterShapePreset::Single);

    emitter.trail = MakeDefaultTrailDesc();

    effect.emitters.push_back(emitter);
    return effect;
}

TrailDesc EffectPanel::MakeDefaultTrailDesc() const
{
    TrailDesc t{};
    t.lifeTime = 0.35f;
    t.widthStart = 80.f;
    t.widthEnd = 2.f;
    t.colorStart = _float4(1.0f, 0.8f, 1.0f, 1.0f);
    t.colorEnd = _float4(1.0f, 0.5f, 1.0f, 0.0f);
    t.widthCurve = EffectCurveType::EaseOut;
    t.alphaCurve = EffectCurveType::EaseOut;
    t.minSegDist = 8.f;

    // 여기만 네가 만든 리본 텍스처 키로 변경
    t.texKey = L"019";

    t.shapeMode = TrailShapeMode::ArcAnalytic;
    t.arcRadius = 400.f;
    t.arcStartDeg = -90.f;
    t.arcEndDeg = 180.f;
    t.arcUseownerCenter = true;
    t.arcCenterOffset = _float3(0.f, 60.f, 0.f);
    return t;
}