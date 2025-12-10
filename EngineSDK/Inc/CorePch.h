#pragma once

// ------ Etc --------------------
#include "Engine_TypeDef.h"
#include "Engine_Enum.h"
#include "Engine_Macro.h"
#include "Engine_Struct.h"
#include "Engine_InputLayout.h"
#include "Engine_Colors.h"

// ----------- Utility -------------
#include "Utility.h"
#include "EffectUtility.h"
#include "Entity.h"
#include "InputUtil.h"
#include "UIData_Enum.h"
#include "UIData_Struct.h"

#include "ParticleData.h"
#include "TrailData.h"
#include "EffectData.h"

#include "CBufferTypes.h"
#include "BattleCamera_Enum.h"
#include "BattleCamera_Struct.h"
#include "CharaAnimData.h"
#include "ActionFxData.h"

#include "ISystem.h"
#include "EngineSystems.h"
#include "SystemRegistry.h"

#include "ComponentPool.h"
#include "Singleton.h"
#include "TagSystem.h"
#include "EntityMgr.h"

#include "MeshUtil.h"
#include "MaterialUtil.h"
#include "InputMgr.h"

// ------------ Data -------------------
#include "RenderProxies.h"
#include "RenderSystemData.h"
#include "RenderScene.h"
#include "NavMeshData.h"
#include "BattleData.h"

#include "RenderSystem.h"

// ------------ System --------------
#include "EntitySystem.h"

#include "AnimDataSystem.h"
#include "ActionAnimRegistry.h"

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
#include "MeshColliderSystem.h"
#include "SkyboxSystem.h"
#include "OrbitCamSystem.h"

// Move 제어 System

#include "MoveIntentSystem.h"
#include "MoveProfileSystem.h"
#include "MoveStateSystem.h"

// AnimSys
#include "FieldAnimSystem.h"
#include "FacingSystem.h"
#include "FacingForceService.h"
#include "FacingBlockService.h"

// Input 제어 
#include "InputGate.h"
#include "IntentCollector.h"
#include "IntentMerger.h"
#include "InputService.h"

#include "BattleTimelineData.h"
#include "BattleSessionData.h"
#include "BattleControllerData.h"
#include "BattleExecutionData.h"

#include "BattleTacticSystem.h"
#include "FieldControllerSystem.h"
#include "BattleTimelineSystem.h"
#include "BattleExecutionSystem.h"

#include "BattleAIControllerSystem.h"
#include "BattleTargetSystem.h"
#include "BattleFormationSystem.h"
#include "BattleCameraDirector.h"
#include "CamRegistry.h"

#include "ActionCamRegistry.h"

// BattleAnimSys
#include "BattleIntroSystem.h"
#include "BattleUIOrchestrator.h"
#include "BattleEventBus.h"
#include "BattleSessionSystem.h"

#include "BattleControllerSystem.h"

#include "EntitySpawner.h"

#include "AssetRegistry.h"
#include "AssetCache.h"

#include "AssetSystem.h"
#include "CharacterDataSystem.h"

#include "FieldOrchestraSystem.h"
#include "BattleOrchestraSystem.h"
#include "GameModeDirectorSystem.h"

// UI
#include "FontSystem.h"
#include "TextLayoutSystem.h"

#include "UIMesh.h"
#include "UIRegistry.h"
#include "UIAnimSystem.h"
#include "UISystem.h"


#include "EffectData.h"

#include "ParticleSystem.h"
#include "ParticleMesh.h"

#include "TrailSystem.h"
#include "TrailMesh.h"

#include "EffectSystem.h"

#include "FieldUIOrchestrator.h"

// ---- Utility ---------------------

#include "GuiMgr.h"
#include "PathMgr.h"
#include "GameInstance.h"
#include "GuiPanel.h"

// ---- EngineComponent ----------------
#include "Device.h"
#include "RenderTargetSystem.h"
#include "Bone.h"
#include "Skeleton.h"

// ---- Resource ----------------------
#include "CBuffer.h"
#include "CBufferBank.h"
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"
#include "Material.h"
#include "Model.h"

// ------- Layer ----------------------
#include "Level.h"
#include "LevelMgr.h"
#include "Renderer.h"
// -----------------------------------------------

using namespace Engine;

extern HWND g_hWnd;