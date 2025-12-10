#pragma once

NS_BEGIN(Engine)

enum class RendererShader
{
	Deferred_Directional, Deferred_Composite, UI, Grid, SkyBox, Particle, Trail, Count
};
static constexpr size_t       rsCount         = ENUM(RendererShader::Count);
static constexpr _uint        ObjCBSizeBytes  = sizeof(ObjCB);
static constexpr _uint        ObjCBConstants  = (ObjCBSizeBytes + 15) / 16;
inline constexpr wstring_view rsKeys[rsCount] = { L"Deferred_Directional", L"Deferred_Composite", L"UI", L"PC", L"P", L"particle", L"trail"};

NS_END