#include "Enginepch.h"
#include "GameInstance.h"

#include "HierachyPanel.h"
#include "InspectorPanel.h"
#include "FpsPanel.h"

GuiMgr::~GuiMgr()
{
#ifdef USE_IMGUI
	ShutDown();
#endif
}

unique_ptr<GuiMgr> GuiMgr::Create(SystemRegistry& registry, EntityMgr& entities)
{
#ifdef USE_IMGUI
	auto instance = make_unique<GuiMgr>(registry, entities);
	instance->Init();
	return instance;
#else
	return make_unique<GuiMgr>(registry, entities);
#endif
}

void GuiMgr::Init()
{
#ifdef USE_IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	// ① Docking / Viewports 활성화
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	// 폰트 설정
	io.Fonts->Clear();
	io.Fonts->AddFontFromFileTTF("../bin/Resources/Fonts/consola.ttf", 18.f, nullptr,  io.Fonts->GetGlyphRangesKorean()); 

	ImGui::StyleColorsDark();
	ApplyDarkTheme();

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) 
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowRounding = 8.0f;       
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	ImGui_ImplWin32_Init(g_hWnd);
	ImGui_ImplDX11_Init(GAME.GetDevice(), GAME.GetContext());
	
	// --------------- Panel  --------------------------
	AddPanel<HierachyPanel>("Hierarchy");
	AddPanel<InspectorPanel>("Inspector");
	AddPanel<FpsPanel>("FPS");
#endif
}

void GuiMgr::ShutDown()
{
#ifdef USE_IMGUI
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif
}

void GuiMgr::Update(float dt)
{
#ifdef USE_IMGUI
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	for (const auto& panel : panels)
		panel->Update(dt);
#endif
}

void GuiMgr::Render()
{
#ifdef USE_IMGUI
	ImGuiWindowFlags dockspaceFlags =
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoNavFocus |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse;

	const ImGuiViewport* vp = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(vp->Pos);
	ImGui::SetNextWindowSize(vp->Size);
	ImGui::SetNextWindowViewport(vp->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);

	ImGui::Begin("MainDockSpace", nullptr, dockspaceFlags);
	ImGui::PopStyleVar(3);

	ImGuiID dockspaceId = ImGui::GetID("MainDockSpaceID");
	ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_PassthruCentralNode;
	ImGui::DockSpace(dockspaceId, ImVec2(0, 0), dockFlags);

	static bool built = false;
	if (!built)
	{
		built = true;

		ImGui::DockBuilderRemoveNode(dockspaceId);
		ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspaceId, vp->Size);

		ImGuiID dock_main = dockspaceId;

		ImGuiID dock_left = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Left, 0.18f, nullptr, &dock_main);
		ImGuiID dock_right = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, 0.26f, nullptr, &dock_main);
		ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Down, 0.22f, nullptr, &dock_main);

		const float kTopBarHeight = 0.06f;
		ImGuiID dock_top = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Up, kTopBarHeight, nullptr, &dock_main);

		ImGuiID dock_top_left = ImGui::DockBuilderSplitNode(dock_top, ImGuiDir_Left, 0.5f, nullptr, &dock_top);
		ImGuiID dock_top_right = dock_top;

		ImGui::DockBuilderDockWindow("Hierarchy", dock_left);
		ImGui::DockBuilderDockWindow("Inspector", dock_right);
		ImGui::DockBuilderDockWindow("Importer", dock_bottom);
		ImGui::DockBuilderDockWindow("MapTool", dock_bottom);
		ImGui::DockBuilderDockWindow("Level", dock_top_left);
		ImGui::DockBuilderDockWindow("FPS", dock_top_right);

		// 탭바 숨김(필요한 곳만)
		if (ImGuiDockNode* n = ImGui::DockBuilderGetNode(dock_top_left))
			n->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
		if (ImGuiDockNode* n = ImGui::DockBuilderGetNode(dock_top_right))
			n->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
		if (ImGuiDockNode* n = ImGui::DockBuilderGetNode(dock_bottom))
			n->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;

		ImGui::DockBuilderFinish(dockspaceId);
	}

	for (const auto& panel : panels)
		panel->DrawPanel();

	ImGui::End();

	ImGui::Render();
	// FLIP_DISCARD 사용시, 백버퍼 RTV를 다시 바인딩
	{
		ID3D11RenderTargetView* rtv = GAME.GetBackBufferRTV();
		ID3D11DepthStencilView* dsv = GAME.GetDSV();
		DC->OMSetRenderTargets(1, &rtv, dsv);
	}

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
#endif
}

LRESULT GuiMgr::ImguiWndProcHandler(_uint msg, WPARAM wParam, LPARAM lParam)
{
#ifdef USE_IMGUI
	if (ImGui_ImplWin32_WndProcHandler(g_hWnd, msg, wParam, lParam))	
		return 1;
#endif 
	return 0;

}

