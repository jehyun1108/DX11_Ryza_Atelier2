#include "pch.h"
#include "Client.h"
#include "MainApp.h"

#define MAX_LOADSTRING 100

HINSTANCE g_hInst;         
HWND g_hWnd;
WCHAR szTitle[MAX_LOADSTRING];           
WCHAR szWindowClass[MAX_LOADSTRING];   

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int nCmdShow)
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //_CrtSetBreakAlloc(1776);
#endif

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow)) return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENT));
    MSG msg;

    auto mainApp = MainApp::Create();

    _float timeAcc{};
    _float dt{};

    constexpr float fps = 1.f / 60;

    GameInstance& game = GameInstance::GetInstance();

    while (true) // Non-Blocking 방식
    { 
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) 
        {
            if (msg.message == WM_QUIT) break;

            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        game.UpdateDt(TIMER::DEFAULT);
        timeAcc += game.GetDt(TIMER::DEFAULT);

        while (timeAcc >= fps)
        {
            mainApp->Update(fps);
            mainApp->Render();
        
            timeAcc -= fps;
        }
    }

    game.ReleaseEngine();

    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLIENT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CLIENT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   g_hInst = hInstance; 

   // 주 모니터의 가로 해상도를 얻어온다.
   int monitorW = GetSystemMetrics(SM_CXSCREEN);

   g_hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       DefaultX, DefaultY,    
       WinX, WinY,    
       nullptr, nullptr, hInstance, nullptr);

   if (!g_hWnd) return FALSE;

   ShowWindow(g_hWnd, nCmdShow);
   UpdateWindow(g_hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    GameInstance& game = GameInstance::GetInstance();

    if (game.ImguiWndProcHandler(message, wParam, lParam))
        return true;

    switch (message)
    {
    case WM_SIZE:
    {
        UINT newX = LOWORD(lParam);
        UINT newY = HIWORD(lParam);

        if (game.IsInited())
            game.OnResize(newX, newY);
    }
    break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

    case WM_INPUT:
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
        game.ProcessWinMsg(message, wParam, lParam);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        DestroyWindow(hWnd);

        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}