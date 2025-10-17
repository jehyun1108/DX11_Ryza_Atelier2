#include "Enginepch.h"

void GuiUtility::Badge(const char* text, const ImVec4& color)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 2));
    ImGui::PushStyleColor(ImGuiCol_Button, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
    ImGui::Button(text);
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
}

void GuiUtility::BeginPanel(const char* title, PanelMode mode, float value)
{
    ImGui::SeparatorText(title);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);

    float height = 0.f;
    switch (mode)
    {
    case PanelMode::Fixed: height = value;                  break;
    case PanelMode::Lines: height = HeightFromLines(value); break;
    case PanelMode::Fill:  height = 0.f;                    break;
    }

    ImGui::BeginChild(title, ImVec2(0, height), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
}

void GuiUtility::EndPanel()
{
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

float GuiUtility::HeightFromLines(float lines)
{
    const ImGuiStyle& style = ImGui::GetStyle();
    return lines * ImGui::GetTextLineHeightWithSpacing() + style.WindowPadding.y * 2.f;
}
