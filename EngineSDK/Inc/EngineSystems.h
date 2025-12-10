#pragma once

NS_BEGIN(Engine)

// Core
class EntityMgr; class RenderSystem;  class InputService; class GameModeDirectorSystem; class Renderer; class InputMgr; class EntitySpawner; class RenderTargetSystem; class ParticleSystem; class EffectSystem; class TrailSystem;

// Data
class AssetSystem;  class CharacterDataSystem; class AnimDataSystem; class ActionAnimRegistry; class GuiMgr; class SoundRegistry; class SoundSystem; class WorldSerializer; class EffectSerializer; class CamSerializer; class ActionFxRegistry;

// Scene
class TransformSystem; class CameraSystem; class LightSystem; class FreeCamSystem; class AnimatorSystem; class FaceSystem; class MouthSystem; class SocketSystem; class ModelSystem; class LayerSystem; class TagSystem; class GridSystem; class PickingSystem; class SelectionSystem; class CollisionSystem; class MeshColliderSystem; class SkyboxSystem; class OrbitCamSystem; class MoveStateSystem; class MoveIntentSystem; class MoveProfileSystem; class JumpSystem; class FacingSystem; class FacingBlockService; class HighlightSystem; class CamRegistry; class FacingForceService; class NavMeshSystem; class LogoOrchestraSystem; class LogoUIOrchestrator;

// UI
class UISystem; class UIRegistry; class UIAnimSystem; class FieldUIOrchestrator; class BattleUIOrchestrator; class PlayerInputPresenter; class BattleFatalDrivePresenter; class BattleHUDPresenter; class BattleTimelinePresenter; class BattleDamagePresenter; class LogoMenuPresenter; class RenderTargetMinimap; class FieldMinimapPresenter; class BattleMinimapPresenter; class UIMinimapSystem; class FontSystem; class TextLayoutSystem; class WorldMapPresenter; class ScreenFadeSystem; class LoadingPresenter; class ScreenDistortionSystem; class BattleTargetHUDPresenter; class BattleBoardPresenter; class DressingRoomPresenter;

// Battle
class BattleOrchestraSystem; class BattleControllerSystem; class BattleIntroSystem; class BattleSessionSystem; class BattleTimelineSystem; class BattleExecutionSystem; class BattleAIControllerSystem; class BattleTargetSystem; class BattleFormationSystem; class BattleCameraDirector; class BattleEventBus;  class BattleTacticSystem; class BattleAttributeSystem; class ActionCamRegistry; class BattleRewardPresenter;

// Field
class FieldAnimSystem; class FieldControllerSystem; class FieldOrchestraSystem;

// ========================================================================================
using Core = TypeList < EntityMgr, RenderSystem, InputService, GameModeDirectorSystem, Renderer, InputMgr, EntitySpawner, RenderTargetSystem, ParticleSystem, EffectSystem, TrailSystem >;

using Data = TypeList< AssetSystem, CharacterDataSystem, AnimDataSystem, ActionAnimRegistry, GuiMgr, SoundRegistry, SoundSystem, WorldSerializer, EffectSerializer, CamSerializer, ActionFxRegistry>;

using Scene = TypeList< TransformSystem, CameraSystem, LightSystem, FreeCamSystem, AnimatorSystem, FaceSystem, MouthSystem, SocketSystem, ModelSystem, LayerSystem, TagSystem, GridSystem, PickingSystem, SelectionSystem, CollisionSystem, MeshColliderSystem, SkyboxSystem, OrbitCamSystem, MoveStateSystem, MoveIntentSystem, MoveProfileSystem, JumpSystem, FacingSystem, FacingBlockService, HighlightSystem, CamRegistry, FacingForceService, NavMeshSystem >;

using UI = TypeList< UISystem, UIRegistry, UIAnimSystem, FieldUIOrchestrator, BattleUIOrchestrator, PlayerInputPresenter, BattleFatalDrivePresenter, BattleHUDPresenter, BattleTimelinePresenter, BattleDamagePresenter, LogoUIOrchestrator, LogoOrchestraSystem, LogoMenuPresenter, RenderTargetMinimap, FieldMinimapPresenter, BattleMinimapPresenter, UIMinimapSystem, FontSystem, TextLayoutSystem, WorldMapPresenter, ScreenFadeSystem, LoadingPresenter, ScreenDistortionSystem, BattleTargetHUDPresenter, BattleBoardPresenter, DressingRoomPresenter>;

using Battle = TypeList< BattleOrchestraSystem, BattleControllerSystem, BattleIntroSystem, BattleSessionSystem, BattleTimelineSystem, BattleExecutionSystem, BattleAIControllerSystem, BattleTargetSystem, BattleFormationSystem, BattleCameraDirector, BattleEventBus, BattleTacticSystem, BattleAttributeSystem, ActionCamRegistry, BattleRewardPresenter >;

using Field = TypeList< FieldAnimSystem, FieldControllerSystem, FieldOrchestraSystem>;
// ===============================================================================================================================================

using AllSystems = typename TypeListCat<
	               typename TypeListCat<Data, Core>::type, 
	               typename TypeListCat<typename TypeListCat<Scene, UI>::type,
	               typename TypeListCat<Battle, Field>::type>::type
>::type;

inline constexpr size_t kSystemCount = TypeListSize<AllSystems>::value;

NS_END