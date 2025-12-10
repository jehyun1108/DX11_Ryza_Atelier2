#pragma once

#include <windows.h>

#include <d3d11.h>
#include <dxgi1_6.h> // dxgifactory5 , √÷Ω≈
#include <DirectXMath.h>
#include <DirectXCollision.h>

//#include "fx11/d3dx11effect.h"
#include "DirectXTK/DDSTextureLoader.h"
#include "DirectXTK/WICTextureLoader.h"
//#include "DirectXTK/ScreenGrab.h"
//#include "DirectXTK/SpriteFont.h"
//#include "DirectXTK/SpriteBatch.h"
#include "DirectXTex/DirectXTex.h"
#include <nlohmann/json.hpp>

//#include "fmod/fmod_studio.hpp"
//#include "fmod/fmod_errors.h"
//#include "fmod/fmod.hpp"
//
//#define USE_EFFECTTOOL
//#define USE_MAPTOOL

//#define USE_IMGUI
//#ifdef _DEBUG
//#pragma push_macro("new")
//#undef new
//#endif
//#ifdef USE_IMGUI 
//#define IMGUI_DEFINE_MATH_OPERATORS
//
//#include "imgui.h"
//#include "imgui_impl_win32.h"
//#include "imgui_impl_dx11.h"
//#include "imgui_internal.h"
//#include "ImGuizmo.h"
//
//#ifdef _DEBUG
//#pragma pop_macro("new")
//#endif
//
//extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
//#endif 

#include <commdlg.h>
#include "d3dcompiler.h"
#include <nfd.h>
#include <wrl.h>
#include <sal.h>
#include <deque>
#include <cctype>
#include <cmath>
#include <fstream>
#include <stdlib.h>
#include <crtdbg.h>
#include <string>
#include <cassert>
#include <type_traits>
#include <typeindex>
#include <map>
#include <vector>
#include <array>
#include <unordered_map>
#include <memory>
#include <functional>
#include <optional>
#include <process.h>
#include <thread>
#include <mutex>
#include <algorithm>
#include <filesystem>
#include <random>
#include <format>
#include <unordered_set>
#include <cstddef>
#include <variant>
#include <numeric>
#include <set>
#include <iostream>
//#include <list>
//#include <utility>
//#include <concepts>
//#include <any>
//#include <string_view> 
//#include <tuple>
//#include <ctype.h>

using namespace DirectX;
using namespace std;
using namespace literals;
using namespace chrono;

using Microsoft::WRL::ComPtr;

#include "CorePch.h"