#pragma once

NS_BEGIN(Engine)

// ======= Layers / Types / Priority ==================================
enum class CamLayer        { Base, Action, Overlay };
enum class CamTrackType    { Follow, Sequence, Scripted, Shake };
enum class CamPriority     { Low, Default, High, Cinematic };

// ====== Blend / Easing / Phase =====================================
enum class CamBlendMode    { CrossFade, Cut };
enum class EaseCurve       { Linear, EaseIn, EaseOut, EaseInOut };
enum class TrackPhase      { Inactive, FadingIn, Sustaining, FadingOut };

// ====== Follow / Anchor ===============================================
enum class AnchorSpace     { World, Target };
enum class TargetBinding   { None, Leader, CurTarget, Attacker, Victim, CustomEntity };

NS_END