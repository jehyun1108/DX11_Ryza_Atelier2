#pragma once

NS_BEGIN(Engine)

enum class CamLayer   
{ 
    Base, Action, Overlay 
};
enum class CamTrackType   
{ 
    Follow, Sequence, Scripted, Shake
};
enum class CamPriority    
{ 
    Low, Default, High, Cinematic 
};
enum class AnchorSpace  
{
    World, Target
};
enum class TargetBinding  
{
    None, Leader, CurTarget, CustomEntity
};
enum class ActionCamAnchor
{
    None, Attacker, Victim, MidPoint,
};

NS_END