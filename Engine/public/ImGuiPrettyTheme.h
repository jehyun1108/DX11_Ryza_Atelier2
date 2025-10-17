#pragma once

inline void ApplyDarkTheme()
{
#ifdef USE_IMGUI
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();

    // 레이아웃/라운딩/여백
    style.WindowRounding    = 8.0f;
    style.ChildRounding     = 7.0f;
    style.PopupRounding     = 7.0f;
    style.FrameRounding     = 7.0f;
    style.GrabRounding      = 6.0f;
    style.ScrollbarRounding = 7.0f;
    style.TabRounding       = 7.0f;
    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 1.0f;
    style.PopupBorderSize   = 1.0f;
    style.WindowPadding     = ImVec2(12, 10);
    style.FramePadding      = ImVec2(10, 6);
    style.ItemSpacing       = ImVec2(10, 8);
    style.ItemInnerSpacing  = ImVec2(6, 6);

    // 컬러 팔레트 (은은한 파스텔 다크)
    auto col = style.Colors;
    const ImVec4 bg0  = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
    const ImVec4 bg1  = ImVec4(0.18f, 0.19f, 0.22f, 1.00f);
    const ImVec4 bg2  = ImVec4(0.22f, 0.23f, 0.27f, 1.00f);
    const ImVec4 ac1  = ImVec4(0.40f, 0.67f, 0.85f, 1.00f); // 포커스/액티브 (파스텔 블루)
    const ImVec4 ac1H = ImVec4(0.48f, 0.75f, 0.92f, 1.00f); // 호버
    const ImVec4 ac2  = ImVec4(0.76f, 0.55f, 0.87f, 1.00f); // 보조 악센트 (라일락)
    const ImVec4 tx0  = ImVec4(0.93f, 0.94f, 0.96f, 1.00f); // 기본 텍스트
    const ImVec4 tx1  = ImVec4(0.75f, 0.78f, 0.82f, 1.00f); // 보조 텍스트

    col[ImGuiCol_Text]         = tx0;
    col[ImGuiCol_TextDisabled] = tx1;
    col[ImGuiCol_WindowBg]     = bg0;
    col[ImGuiCol_ChildBg]      = ImVec4(bg0.x, bg0.y, bg0.z, 0.0f);
    col[ImGuiCol_PopupBg]      = bg1;

    col[ImGuiCol_Border]       = ImVec4(0.25f, 0.27f, 0.32f, 1.00f);
    col[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);

    col[ImGuiCol_FrameBg]        = bg2;
    col[ImGuiCol_FrameBgHovered] = ImVec4((bg2.x + ac1H.x) * 0.5f, (bg2.y + ac1H.y) * 0.5f, (bg2.z + ac1H.z) * 0.5f, 1.00f);
    col[ImGuiCol_FrameBgActive]  = ac1;

    col[ImGuiCol_TitleBg]          = bg0;
    col[ImGuiCol_TitleBgActive]    = bg1;
    col[ImGuiCol_TitleBgCollapsed] = bg0;
     
    col[ImGuiCol_MenuBarBg] = bg1;

    col[ImGuiCol_ScrollbarBg]          = bg0;
    col[ImGuiCol_ScrollbarGrab]        = ImVec4(0.35f, 0.38f, 0.44f, 1.00f);
    col[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.45f, 0.48f, 0.55f, 1.00f);
    col[ImGuiCol_ScrollbarGrabActive]  = ac1;

    col[ImGuiCol_CheckMark] = ac1;

    col[ImGuiCol_SliderGrab]       = ac1;
    col[ImGuiCol_SliderGrabActive] = ac1H;

    col[ImGuiCol_Button]        = ImVec4(0.31f, 0.50f, 0.66f, 0.85f);
    col[ImGuiCol_ButtonHovered] = ac1H;
    col[ImGuiCol_ButtonActive]  = ac1;

    col[ImGuiCol_Header]        = ImVec4(0.28f, 0.40f, 0.55f, 0.65f);
    col[ImGuiCol_HeaderHovered] = ImVec4(0.32f, 0.52f, 0.70f, 0.80f);
    col[ImGuiCol_HeaderActive]  = ac1;

    col[ImGuiCol_Separator]        = ImVec4(0.28f, 0.30f, 0.35f, 1.00f);
    col[ImGuiCol_SeparatorHovered] = ac1H;
    col[ImGuiCol_SeparatorActive]  = ac1;

    col[ImGuiCol_ResizeGrip]        = ImVec4(0.60f, 0.60f, 0.60f, 0.25f);
    col[ImGuiCol_ResizeGripHovered] = ac1H;
    col[ImGuiCol_ResizeGripActive]  = ac1;

    col[ImGuiCol_Tab]                = ImVec4(0.25f, 0.35f, 0.48f, 1.00f);
    col[ImGuiCol_TabHovered]         = ImVec4(0.34f, 0.52f, 0.72f, 1.00f);
    col[ImGuiCol_TabActive]          = ImVec4(0.30f, 0.44f, 0.61f, 1.00f);
    col[ImGuiCol_TabUnfocused]       = ImVec4(0.20f, 0.26f, 0.33f, 1.00f);
    col[ImGuiCol_TabUnfocusedActive] = ImVec4(0.22f, 0.32f, 0.44f, 1.00f);

    // 도킹 노드 컬러 (중앙 투명일 땐 큰 영향 X, 그래도 통일감 유지)
    col[ImGuiCol_DockingPreview] = ImVec4(ac2.x, ac2.y, ac2.z, 0.30f);
    col[ImGuiCol_DockingEmptyBg] = bg0;

    // 선택 & 드래그
    col[ImGuiCol_NavHighlight]   = ac2;
    col[ImGuiCol_DragDropTarget] = ImVec4(0.95f, 0.60f, 0.20f, 0.90f);
#endif 
}