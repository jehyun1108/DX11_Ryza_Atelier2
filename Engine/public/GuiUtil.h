#pragma once

NS_BEGIN(Engine)

struct ScopedCompact
{
    explicit ScopedCompact(bool forTable = false,
        ImVec2 itemSpacing = ImVec2(6, 2),
        ImVec2 framePadding = ImVec2(6, 2),
        ImVec2 cellPadding = ImVec2(6, 2),
        ImVec2 windowPadding = ImVec2(6, 4))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, itemSpacing);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, framePadding);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, windowPadding);
        count = 3;

#if defined(IMGUI_VERSION_NUM) && (IMGUI_VERSION_NUM >= 18900)
        if (forTable)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cellPadding);
            ++count;
        }
#endif
    }

    ~ScopedCompact() { ImGui::PopStyleVar(count); }

    int count{};
};


NS_END