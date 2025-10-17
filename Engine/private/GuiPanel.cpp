#include "Enginepch.h"
#include "GuiPanel.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

void GuiPanel::DrawPanel()
{
#ifdef USE_IMGUI
	if (!isActive) return;
	ImGui::Begin(title.c_str(), &isActive);
	Draw();
	ImGui::End();
#endif
}
