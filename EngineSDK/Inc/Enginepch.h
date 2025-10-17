#pragma once

#include <windows.h>

#include <d3d11.h>
#include <dxgi1_6.h> // dxgifactory5 , √÷Ω≈
#include <DirectXMath.h>
#include <DirectXCollision.h>

//#include "fx11/d3dx11effect.h"
#include "DirectXTK/DDSTextureLoader.h"
#include "DirectXTK/WICTextureLoader.h"
#include "DirectXTK/ScreenGrab.h"
#include "DirectXTK/SpriteFont.h"
#include "DirectXTK/SpriteBatch.h"
#include "DirectXTex/DirectXTex.h"

#include <nlohmann/json.hpp>

//#ifdef _DEBUG
#define USE_IMGUI
//#endif

//--------- ImGui ---------------------
#ifdef USE_IMGUI 
#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "imgui_internal.h"
#include "ImGuizmo.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif 

#include <commdlg.h>

#include "d3dcompiler.h"

#include <nfd.h>
#include <wrl.h>

#include <cctype>
#include <cmath>
#include <string_view> 
#include <fstream>
#include <ctype.h>
#include <stdlib.h>
#include <crtdbg.h>
#include <string>
#include <cassert>
#include <type_traits>
#include <typeindex>
#include <map>
#include <vector>
#include <list>
#include <array>
#include <unordered_map>
#include <sal.h>
#include <memory>
#include <utility>
#include <any>
#include <concepts>
#include <functional>
#include <tuple>
#include <optional>
#include <process.h>
#include <thread>
#include <mutex>
#include <algorithm>
#include <iostream>
#include <filesystem>
#include <random>
#include <set>
#include <format>
#include <unordered_set>
#include <cstddef>

using namespace DirectX;
using namespace std;
using namespace literals;
using namespace chrono;

using Microsoft::WRL::ComPtr;

// ------ Etc --------------------
#include "Engine_TypeDef.h"
#include "Engine_Enum.h"
#include "Engine_Macro.h"
#include "Engine_Struct.h"
#include "Engine_ShaderStruct.h"
#include "Engine_Colors.h"

// ----------- Utility -------------
#include "Utility.h"

#include "Entity.h"

#include "SystemRegistry.h"
#include "ComponentPool.h"
#include "Singleton.h"
#include "TagSystem.h"
#include "EntityMgr.h"

// ------------ Data -------------------
#include "RenderProxies.h"
#include "RenderSystemData.h"
#include "RenderScene.h"
#include "RenderData.h"

#include "RenderSystem.h"

// ------------ System --------------
#include "EntitySystem.h"

#include "TransformSystem.h"
#include "AnimatorSystem.h"
#include "MouthSystem.h"
#include "FaceSystem.h"
#include "SocketSystem.h"
#include "CameraSystem.h"
#include "FreeCamSystem.h"
#include "LightSystem.h"
#include "ModelSystem.h"
#include "LayerSystem.h"
#include "GridSystem.h"
#include "PickingSystem.h"
#include "SelectionSystem.h"
#include "CollisionSystem.h"
#include "HighlightSystem.h"

#include "EntitySpawner.h"

#include "AssetRegistry.h"
#include "AssetCache.h"

#include "AssetSystem.h"

// ---- Utility ---------------------

#include "GuiMgr.h"
#include "InputMgr.h"
#include "PathMgr.h"
#include "GameInstance.h"
#include "GuiPanel.h"

// ---- EngineComponent ----------------
#include "Device.h"
#include "Bone.h"
#include "Skeleton.h"

// ---- Resource ----------------------
#include "CBuffer.h"
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"
#include "Material.h"
#include "Model.h"

// --------- ResourceSystem ---------------


// ------- Layer ----------------------
#include "Level.h"
#include "LevelMgr.h"
#include "Renderer.h"
// -----------------------------------------------

using namespace Engine;

extern HWND g_hWnd;