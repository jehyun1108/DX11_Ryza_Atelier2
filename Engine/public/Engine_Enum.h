#pragma once

namespace Engine
{
    enum class LAYER            { CAMERA, TERRAIN, MAPOBJ, PLAYER, SOCKET, MONSTER, EFFECT, UI, END};
	enum class TIMER            { DEFAULT, FPS60, FPS144, COUNT };
	enum class WINMODE          { FULL, WIN, END};
	enum class STATE            { RIGHT, UP, LOOK,  END };
	enum class MOVE             { FORWARD, BACK, LEFT, RIGHT, UP, DOWN, END };
    enum class CBUFFERSLOT      { CAMERA, OBJ, LIGHT, BONE, END};
    enum class RASTERIZER       { CULLBACK, CULLFRONT, CULLNONE, WIREFRAME, END};
    enum class BLENDSTATE       { Opaque, ALPHABLEND, ADDITIVE, END };
    enum class DEPTHSTATE       { DEFAULT, DEPTHSTENCILWRITE, DEPTHSTENCIL_NOEQUAL, NO_DEPTHTEST, NO_DEPTHWRITE, NO_DEPTHWRITE_LESSEQUAL, END};
    enum class SAMPLER          { POINT, LINEAR, ANISOTROPIC, SHADOW, END};
    enum class LIGHT            { DIRECTIONAL, POINT, SPOT, END};
    enum class INTERPOLATION    { LINEAR, STEP, CUBICSPLINE, END };
    enum class ANIMTYPE         { ONCE, LOOP, PINGPONG, END };
    enum class ANIMBLEND        { OVERRIDE, ADDITIVE, END};
    enum class TextureColorSpace
    {
        sRGB, // Albedo, UI, Sky 등 "눈에 보이는 색"
        Linear      // Normal, Mask, Depth 등 수치데이터
    };
    enum class VertexLayoutID  
    { 
        Unknown, 
        PUV,
        PNU,        // RectMesh / Sprite / FullScreen Quad (Pos + UV)
        PNUTan,     // Static  (Pos, Normal, UV, Tangent)
        PNUTanSkin, // Skinned (PNUTan + skinning)      
    };
    enum class TEXSLOT
    {
        ALBEDO,     // t0 레지스터
        NORMAL,     // t1
        ROUGHNESS,  // t2
        METALIC,	// t3
        AO,         // t4
        EMISSIVE,   // t5
        END,
    };

    enum class SHADER : UINT8
    {
        NONE = 0,
        VS = (1 << 0), // 0000 0001
        PS = (1 << 1), // 0000 0010
        GS = (1 << 2), // 0000 0100
        HS = (1 << 3), // 0000 1000
        DS = (1 << 4), // 0001 0000
        CS = (1 << 5), // 0010 0000
        ALL = 0xFF
    };

    inline constexpr SHADER operator|(SHADER a, SHADER b) {
        return static_cast<SHADER>(static_cast<UINT8>(a) | static_cast<UINT8>(b));
    }
    inline constexpr bool operator&(SHADER a, SHADER b) {
        return (static_cast<UINT8>(a) & static_cast<UINT8>(b)) != 0;
    }
}