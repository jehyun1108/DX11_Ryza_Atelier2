#pragma once

#include "EffectPanelData.h"

NS_BEGIN(Importer)

class EffectPanel final : public GuiPanel
{
public:
	EffectPanel(string title, SystemRegistry& registry, EntityID* selected);

public:
	void Draw() override;

private:
	void DrawAssetSection();
	void DrawInfoSection();
	void DrawPreviewSettings();
	void DrawPreviewPlay();

	void NewEffect();
	void OpenEffect();
	void SaveEffect();
	void PlayPreview();

	EffectArchetype MakeDefaultEffect() const;
	TrailDesc MakeDefaultTrailDesc() const;

private:
	EffectEditorState state;

private:
	EffectSystem*     effectSys{};
	EffectSerializer* effectSer{};
	CameraSystem*     camSys{};
	TransformSystem*  tfSys{};
	InputService*     input{};
	AssetSystem*      assets{};
};

NS_END