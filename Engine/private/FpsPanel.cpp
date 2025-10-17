#include "Enginepch.h"
#include "FpsPanel.h"

void FpsPanel::Draw()
{
#ifdef USE_IMGUI

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

#endif
}
