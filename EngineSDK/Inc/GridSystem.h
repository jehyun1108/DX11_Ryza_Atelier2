#pragma once

#include "GridData.h"

NS_BEGIN(Engine)

class ENGINE_DLL GridSystem : public EntitySystem<GridData>, public IGuiRenderable
{
public:
	explicit GridSystem(SystemRegistry& registry) : EntitySystem(registry) {}

	// 생성/설정
	Handle Create(EntityID owner);

	void   SetParams(Handle handle, const GridParams& param);
	const  GridParams& GetParams(Handle handle) const;
	const  GridParams* TryGetParams(Handle handle) const;

	const GridParams* TryGetParamsByOwner(EntityID owner) const;
	const GridParams& GetParamsByOwner(EntityID owner) const;

	void   SetCellSize(Handle handle, float size);
	void   SetOrigin(Handle handle, const _float3& origin);
	void   SetCellCount(Handle handle, int countX, int countZ);
	void   SetMajorEvery(Handle handle, int n);
	void   EnableHover(Handle handle, bool enabled);

	// Hover
	void SetHover(Handle handle, const GridCoord& cell, bool isValid);
	void SetHoverArea(Handle handle, const CellRect& rect, bool isValid);
	bool SetHoverForAABB(Handle handle, const _float3& minWorld, const _float3& maxWorld, bool isValid);

	// PipeLine
	void Build(Handle handle); 
	void Render(Handle handle, ID3D11DeviceContext* context);
	void Update(float dt);

	void RenderLines(Handle handle, ID3D11DeviceContext* context);
	void RenderHover(Handle handle, ID3D11DeviceContext* context);
	void RenderAllLines(ID3D11DeviceContext* context);
	void RenderAllHover(ID3D11DeviceContext* context);	

	// Coord/Picking
	GridCoord WorldToCell(Handle handle, const _float3& world) const;
	_float3   CellToWorldCenter(Handle handle, const GridCoord& cell) const;
	void      CellBounds(Handle handle, const GridCoord& cell, _float3& outMin, _float3& outMax) const;
	bool      RayCastToPlane(Handle handle, const _vec& rayOrigin, const _vec& rayDir, _float3& outHit) const;
	bool      PickCell(Handle handle, const _vec& rayOrigin, const _vec& rayDir, GridCoord& outCell, _float3& outHit) const;

	void      RenderGui(EntityID id) override;

private:
	void BuildLineVertices(const GridParams& params, vector<VertexColor>& out) const;
	void BuildHoverVertices(const GridParams& params, bool hoverValid, const CellRect& rect,
		vector<VertexColor>& fill, vector<VertexColor>& border) const;

	// AABB/Cell
	CellRect WorldAABBToCellRect(const GridParams& params, const _float3& minWorld, const _float3& maxWorld) const;
	CellRect ClampRectToGrid(const GridParams& params, const CellRect& rect) const;
	bool     IsRectInsideGrid(const GridParams& params, const CellRect& rect) const;

	// GPU Utility
	void EnsureDynamicVB(ComPtr<ID3D11Buffer>& buffer, _uint requiredBytes, _uint& capacityBytes) const;
	void UploadVB(ID3D11Buffer* buffer, const void* srcData, _uint bytes) const;
};

NS_END