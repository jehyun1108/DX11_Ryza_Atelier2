#include "Enginepch.h"

void NavMeshSystem::SetRenderParams(Handle handle, const NavMeshRenderParams& params)
{
}

const NavMeshRenderParams* NavMeshSystem::GetRenderParams() const
{
	return nullptr;
}

void NavMeshSystem::SetPreviewBoundary(Handle handle, const vector<BoundaryEdge>& edges)
{
}

void NavMeshSystem::ClearPreviewBoundary(Handle handle)
{
}

void NavMeshSystem::MarkDirty(Handle handle)
{
}

void NavMeshSystem::MarkDirtyByOwner(EntityID owner)
{
}

void NavMeshSystem::Build(Handle handle)
{

}

void NavMeshSystem::Render(Handle handle, ID3D11DeviceContext* context)
{

}

void NavMeshSystem::RenderAll(ID3D11DeviceContext* context)
{

}

void NavMeshSystem::Update(float dt)
{

}

void NavMeshSystem::RenderGui(EntityID id)
{
#ifdef USE_IMGUI


#endif
}
