#include "pch.h"
#include "Exporter.h"

#include "MainApp.h"

#define MAX_LOADSTRING 100

HWND g_hWnd;
HINSTANCE g_hInst;                                
WCHAR szTitle[MAX_LOADSTRING];                 
WCHAR szWindowClass[MAX_LOADSTRING];          

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_EXPORTER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    if (!InitInstance(hInstance, nCmdShow)) return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_EXPORTER));
    
    auto mainApp = MainApp::Create();

    constexpr float fps    = 1.f / 144.f;
    constexpr float maxAcc = 0.25f;

    MSG msg{};
    float acc = 0.f;

    GameInstance& game = GameInstance::GetInstance();

    while(true)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) break;

            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        game.UpdateDt(TIMER::DEFAULT);
        float dt = game.GetDt(TIMER::DEFAULT);
        acc += dt;
        if (acc > maxAcc)
            acc = maxAcc;

        game.BeginFrame(dt);

        while (acc >= fps)
        {
            mainApp->Update(fps);
            acc -= fps;
        }

        game.EndFrame();
        mainApp->Render();
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EXPORTER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_EXPORTER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    g_hInst = hInstance;

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

    if (game.IsInited())
    {
        game.ImguiWndProcHandler(message, wParam, lParam);
        game.ProcessWinMsg(message, wParam, lParam);
    }

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

    case WM_DESTROY:
        PostQuitMessage(0);
        exit(0);
        break;

    default:
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}