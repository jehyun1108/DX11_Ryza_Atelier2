#pragma once

NS_BEGIN(Engine)

// Core
class EntityMgr; class RenderSystem;  class InputService; class GameModeDirectorSystem; class Renderer; class InputMgr; class EntitySpawner;

// Data
class AssetSystem;  class CharacterDataSystem; class AnimDataSystem; class ActionAnimRegistry; class GuiMgr;

// Scene
class TransformSystem; class CameraSystem; class LightSystem; class FreeCamSystem; class AnimatorSystem; class FaceSystem; class MouthSystem; class SocketSystem; class ModelSystem; class LayerSystem; class TagSystem; class GridSystem; class PickingSystem; class SelectionSystem; class CollisionSystem; class MeshColliderSystem; class SkyboxSystem; class OrbitCamSystem; class MoveStateSystem; class MoveIntentSystem; class MoveProfileSystem; class JumpSystem; class FacingSystem; class FacingBlockService; class HighlightSystem; class CamRegistry; class FacingForceService;

// UI
class UISystem; class UIRegistry; class UIAnimSystem; class FieldUIOrchestrator; class BattleUIOrchestrator;

// Battle
class BattleOrchestraSystem; class BattleControllerSystem; class BattleIntroSystem; class BattleSessionSystem; class BattleTimelineSystem; class BattleExecutionSystem; class BattleAIControllerSystem; class BattleTargetSystem; class BattleFormationSystem; class BattleCameraDirector; class BattleEventBus; class BattleTimelinePresenter; 

// Field
class FieldAnimSystem; class FieldControllerSystem; class FieldOrchestraSystem;

// Utility
class GuiMgr;

// ========================================================================================
using Core = TypeList <
	EntityMgr, RenderSystem, InputService, GameModeDirectorSystem, Renderer, InputMgr, EntitySpawner
>;

using Data = TypeList<
	AssetSystem, CharacterDataSystem, AnimDataSystem, ActionAnimRegistry, GuiMgr
>;

using Scene = TypeList<
	TransformSystem, CameraSystem, LightSystem, FreeCamSystem, AnimatorSystem, FaceSystem, MouthSystem, SocketSystem, ModelSystem, LayerSystem, TagSystem, GridSystem, PickingSystem, SelectionSystem, CollisionSystem, MeshColliderSystem, SkyboxSystem, OrbitCamSystem, MoveStateSystem, MoveIntentSystem, MoveProfileSystem, JumpSystem, FacingSystem, FacingBlockService, HighlightSystem, CamRegistry, FacingForceService
>;

using UI = TypeList<
	UISystem, UIRegistry, UIAnimSystem, FieldUIOrchestrator, BattleUIOrchestrator
>;

using Battle = TypeList<
	BattleOrchestraSystem, BattleControllerSystem, BattleIntroSystem, BattleSessionSystem, BattleTimelineSystem, BattleExecutionSystem, BattleAIControllerSystem, BattleTargetSystem, BattleFormationSystem, BattleCameraDirector, BattleEventBus, BattleTimelinePresenter
>;

using Field = TypeList<
	FieldAnimSystem, FieldControllerSystem, FieldOrchestraSystem
>;

NS_END