#pragma once

#include "GuiUtil.h"

NS_BEGIN(Engine)

enum class PanelMode {Fixed, Lines, Fill};

class ENGINE_DLL GuiUtility
{
public:
	static void Badge(const char* text, const ImVec4& color);
	static void BeginPanel(const char* title, PanelMode mode = PanelMode::Lines, float value = 0.f);
	static void EndPanel();

private:
	static float HeightFromLines(float lines);
};

NS_END