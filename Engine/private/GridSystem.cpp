#include "Enginepch.h"

Handle GridSystem::Create(EntityID owner)
{
	Handle handle = CreateComp(owner);
	if (auto grid = Get(handle))
	{
		grid->params            = {};
		grid->params.cellSize   = max(1e-6f, grid->params.cellSize);
		grid->params.majorEvery = max(1, grid->params.majorEvery);
		grid->linesDirty        = grid->hoverDirty = true;
		grid->hoverValid        = false;
		grid->hoverRect         = {};
		grid->vbLines.Reset(); grid->vbHoverFill.Reset(); grid->vbHoverBorder.Reset();
		grid->vbLinesCapacityBytes = grid->vbHoverFillCapacityBytes = grid->vbHoverBorderCapacityBytes = 0;
	}
	return handle;
}

//------------------ Set -----------------------
void GridSystem::SetParams(Handle handle, const GridParams& param)
{
	if (auto grid = Get(handle))
	{
		GridParams newParam = param;
		newParam.cellSize = max(1e-6f, newParam.cellSize);
		newParam.majorEvery = max(1, newParam.majorEvery);

		const auto& oParam = grid->params;
		bool linesChange =
			!Utility::NearlyEqual(oParam.cellSize, newParam.cellSize) ||
			oParam.cellCountX != newParam.cellCountX ||
			oParam.cellCountZ != newParam.cellCountZ ||
			!Utility::NearlyEqual(oParam.origin.x, newParam.origin.x) ||
			!Utility::NearlyEqual(oParam.origin.y, newParam.origin.y) ||
			!Utility::NearlyEqual(oParam.origin.z, newParam.origin.z) ||
			oParam.majorEvery != newParam.majorEvery ||
			oParam.showMajor != newParam.showMajor ||
			oParam.showMinor != newParam.showMinor;

		bool hoverChange =
			linesChange ||
			oParam.showHover != newParam.showHover ||
			memcmp(&oParam.colorHover, &newParam.colorHover, sizeof(oParam.colorHover)) != 0 ||
			memcmp(&oParam.colorHoverBorder, &newParam.colorHoverBorder, sizeof(oParam.colorHoverBorder)) != 0;

		grid->params = newParam;
		grid->linesDirty = linesChange;
		grid->hoverDirty = hoverChange;
	}
}

const GridParams* GridSystem::TryGetParams(Handle handle) const
{
	if (auto grid = Get(handle))
		return &grid->params;
	return nullptr;
}

const GridParams* GridSystem::TryGetParamsByOwner(EntityID owner) const
{
	Handle handle{};
	if (const auto grid = GetByOwner(owner, &handle))
		return &grid->params;
	return nullptr;
}

const GridParams& GridSystem::GetParamsByOwner(EntityID owner) const
{
	Handle handle{};
	const auto grid = GetByOwner(owner, &handle);
	assert(grid && "GridSystem::GetParamsByOwner - owner has no grid");
	return grid->params;
}

const GridParams& GridSystem::GetParams(Handle handle) const
{
	const auto grid = Get(handle);
	assert(grid && "GridSystem::GetParams - invalid handle");
	return grid->params;
}

void GridSystem::SetCellSize(Handle handle, float size)
{
	if (auto grid = Get(handle))
	{
		float newSize = max(1e-6f, size);
		if (grid->params.cellSize != newSize)
		{
			grid->params.cellSize = newSize;
			grid->linesDirty = true;
			grid->hoverDirty = true;
		}
	}
}

void GridSystem::SetOrigin(Handle handle, const _float3& origin)
{
	if (auto grid = Get(handle))
	{
		if (memcmp(&grid->params.origin, &origin, sizeof(_float3)) != 0)
		{
			grid->params.origin = origin;
			grid->linesDirty = true;
			grid->hoverDirty = true;
		}
	}
}

void GridSystem::SetCellCount(Handle handle, int countX, int countZ)
{
	if (auto grid = Get(handle))
	{
		int numX = max(1, countX);
		int numZ = max(1, countZ);
		if (grid->params.cellCountX != numX || grid->params.cellCountZ != numZ)
		{
			grid->params.cellCountX = numX;
			grid->params.cellCountZ = numZ;
			grid->linesDirty = true;
			grid->hoverDirty = true;
		}
	}
}

void GridSystem::SetMajorEvery(Handle handle, int n)
{
	if (auto grid = Get(handle))
	{
		int numEvery = max(1, n);
		if (grid->params.majorEvery != numEvery)
		{
			grid->params.majorEvery = numEvery;
			grid->linesDirty = true;
		}
	}
}

void GridSystem::EnableHover(Handle handle, bool enabled)
{
	if (auto grid = Get(handle))
	{
		if (grid->params.showHover != enabled)
		{
			grid->params.showHover = enabled;
			grid->hoverDirty = true;
		}
	}
}

// ------------- Hover ---------------------
void GridSystem::SetHover(Handle handle, const GridCoord& cell, bool isValid)
{
	if (auto grid = Get(handle))
	{
		CellRect rect{ cell.x, cell.x, cell.z, cell.z };
		grid->hoverRect  = rect;
		grid->hoverValid = isValid;
		grid->hoverDirty = true;
	}
}

void GridSystem::SetHoverArea(Handle handle, const CellRect& rect, bool isValid)
{
	if (auto grid = Get(handle))
	{
		grid->hoverRect  = rect;
		grid->hoverValid = isValid && !rect.Empty();
		grid->hoverDirty = true;
	}
}

bool GridSystem::SetHoverForAABB(Handle handle, const _float3& minWorld, const _float3& maxWorld, bool isValid)
{
	if (auto grid = Get(handle))
	{
		CellRect rect    = WorldAABBToCellRect(grid->params, minWorld, maxWorld);
		CellRect clamped = ClampRectToGrid(grid->params, rect);
		bool fullyInside = IsRectInsideGrid(grid->params, rect);
		SetHoverArea(handle, clamped, isValid);
		return fullyInside;
	}
	return false;
}

// ------------------ Pipeline --------------
void GridSystem::Build(Handle handle)
{
	auto grid = Get(handle);
	if (!grid) return;

	auto uploadVertices = [this](ComPtr<ID3D11Buffer>& vb, _uint& capacityBytes, const vector<VertexColor>& vertices, _uint& outVertexCount)
		{
			outVertexCount = static_cast<_uint>(vertices.size());

			if (outVertexCount == 0)
			{
				vb.Reset();
				capacityBytes = 0;
				return;
			}

			const _uint rawBytes = static_cast<_uint>(sizeof(VertexColor) * outVertexCount); 
			const _uint gpuBytes = Utility::AlignTo16Bytes(rawBytes);                       

			EnsureDynamicVB(vb, gpuBytes, capacityBytes);
			UploadVB(vb.Get(), vertices.data(), rawBytes);
		};

	// -------- Lines --------
	if (grid->linesDirty)
	{
		vector<VertexColor> lineVertices;
		BuildLineVertices(grid->params, lineVertices);

		uploadVertices(grid->vbLines, grid->vbLinesCapacityBytes, lineVertices, grid->lineVertexCount);

		grid->linesDirty = false;
	}

	// -------- Hover --------
	if (grid->hoverDirty)
	{
		vector<VertexColor> fillVertices, borderVertices;
		BuildHoverVertices(grid->params, grid->hoverValid && grid->params.showHover,grid->hoverRect,fillVertices, borderVertices);

		uploadVertices(grid->vbHoverFill,grid->vbHoverFillCapacityBytes,fillVertices,grid->hoverFillVertexCount);
		uploadVertices(grid->vbHoverBorder,grid->vbHoverBorderCapacityBytes, borderVertices, grid->hoverBorderVertexCount);

		grid->hoverDirty = false;
	}
}

void GridSystem::RenderLines(Handle handle, ID3D11DeviceContext* context)
{
	auto grid = Get(handle);
	if (!grid) return;
	if (!(grid->vbLines && grid->lineVertexCount > 0)) return;

	const _uint stride = sizeof(VertexColor);
	const _uint offset = 0;
	ID3D11Buffer* buffer = grid->vbLines.Get();
	context->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
	context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	context->Draw(grid->lineVertexCount, 0);
}

void GridSystem::RenderHover(Handle handle, ID3D11DeviceContext* context)
{
	auto grid = Get(handle);
	if (!grid) return;
	if (!(grid->params.showHover && grid->hoverValid)) return;

	const _uint stride = sizeof(VertexColor);
	const _uint offset = 0;

	if (grid->vbHoverFill && grid->hoverFillVertexCount > 0)
	{
		ID3D11Buffer* buffer = grid->vbHoverFill.Get();
		context->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->Draw(grid->hoverFillVertexCount, 0);
	}
	if (grid->vbHoverBorder && grid->hoverBorderVertexCount > 0)
	{
		ID3D11Buffer* buffer = grid->vbHoverBorder.Get();
		context->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		context->Draw(grid->hoverBorderVertexCount, 0);
	}
}

void GridSystem::RenderAllLines(ID3D11DeviceContext* context)
{
	ForEachAliveEx([&](Handle handle, EntityID, GridData&) { RenderLines(handle, context);});
}

void GridSystem::RenderAllHover(ID3D11DeviceContext* context)
{
	ForEachAliveEx([&](Handle handle, EntityID, GridData&) { RenderHover(handle, context);});
}

void GridSystem::Render(Handle handle, ID3D11DeviceContext* context)
{
	RenderLines(handle, context);
	RenderHover(handle, context);
}

void GridSystem::Update(float dt)
{
	ForEachAliveEx([&](Handle handle, EntityID, GridData& data) 
		{
			if (data.linesDirty || data.hoverDirty)
				Build(handle);
		});
}

// -------------- Utility ---------------------
GridCoord GridSystem::WorldToCell(Handle handle, const _float3& world) const
{
	const auto grid = Get(handle);
	if (!grid) return {};
	const auto& param = grid->params;
	const float nx = (world.x - param.origin.x) / param.cellSize;
	const float nz = (world.z - param.origin.z) / param.cellSize;
	return GridCoord{ (int)floorf(nx), (int)floorf(nz) };
}

_float3 GridSystem::CellToWorldCenter(Handle handle, const GridCoord& cell) const
{
	const auto grid = Get(handle);
	if (!grid) return {};
	const auto& param = grid->params;
	const float minX = param.origin.x + cell.x * param.cellSize;
	const float minZ = param.origin.z + cell.z * param.cellSize;
	return _float3{ minX + param.cellSize * 0.5f, param.origin.y, minZ + param.cellSize * 0.5f };
}

void GridSystem::CellBounds(Handle handle, const GridCoord& cell, _float3& outMin, _float3& outMax) const
{
	const auto grid = Get(handle);
	if (!grid)
	{
		outMin = {};
		outMax = {};
		return;
	}
	const auto& param = grid->params;
	const float minX  = param.origin.x + cell.x * param.cellSize;
	const float minZ  = param.origin.z + cell.z * param.cellSize;
	outMin = _float3{ minX, param.origin.y, minZ };
	outMax = _float3{ minX + param.cellSize, param.origin.y, minZ + param.cellSize };
}

bool GridSystem::RayCastToPlane(Handle handle, const _vec& rayOrigin, const _vec& rayDir, _float3& outHit) const
{
	const auto grid     = Get(handle);
	if (!grid) return false;

	const float planeY  = grid->params.origin.y;
	const float originY = XMVectorGetY(rayOrigin);
	const float dirY    = XMVectorGetY(rayDir);
	if (fabsf(dirY) < 1e-6f)
		return false;
	const float t = (planeY - originY) / dirY;
	if (t < 0.f)
		return false;
	const auto hit = XMVectorMultiplyAdd(rayDir, XMVectorReplicate(t), rayOrigin);
	outHit = { XMVectorGetX(hit), XMVectorGetY(hit), XMVectorGetZ(hit) };
	return true;
}

bool GridSystem::PickCell(Handle handle, const _vec& rayOrigin, const _vec& rayDir, GridCoord& outCell, _float3& outHit) const
{
	if (!RayCastToPlane(handle, rayOrigin, rayDir, outHit)) return false;
	outCell = WorldToCell(handle, outHit);
	const auto grid = Get(handle);
	if (!grid) return false;
	const auto& param = grid->params;
	const int halfX = param.cellCountX / 2;
	const int halfZ = param.cellCountZ / 2;
	return (outCell.x >= -halfX && outCell.x < halfX) && (outCell.z >= -halfZ && outCell.z < halfZ);
}

// ------------- Util --------------------
void GridSystem::BuildLineVertices(const GridParams& params, vector<VertexColor>& out) const
{
	out.clear();
	const float size    = params.cellSize;
	const float originX = params.origin.x;
	const float originY = params.origin.y;
	const float originZ = params.origin.z;
	const int halfX     = params.cellCountX / 2;
	const int halfZ     = params.cellCountZ / 2;

	const int linesVertical = params.cellCountX + 1;
	const int linesHorizontal = params.cellCountZ + 1;
	out.reserve((linesVertical + linesHorizontal) * 2);

	auto push = [&](const _float3& a, const _float3& b, const _float4& c) 
		{
			out.push_back({ a, c });
			out.push_back({ b, c });
		};

	// 세로
	for (int idxX = -halfX; idxX <= halfX; ++idxX)
	{
		const bool isMajor = (params.majorEvery > 0) ? ((idxX % params.majorEvery) == 0) : false;
		if ((isMajor && !params.showMajor) || (!isMajor && !params.showMinor)) continue;

		const _float4 col = isMajor ? params.colorMajor : params.colorMinor;
		const float x = originX + idxX * size;
		push({ x, originY, originZ - halfZ * size }, { x, originY, originZ + halfZ * size }, col);
	}
	// 가로
	for (int idxZ = -halfZ; idxZ <= halfZ; ++idxZ)
	{
		const bool isMajor = (params.majorEvery > 0) ? ((idxZ % params.majorEvery) == 0) : false;
		if ((isMajor && !params.showMajor) || (!isMajor && !params.showMinor)) continue;

		const _float4 col = isMajor ? params.colorMajor : params.colorMinor;
		const float z = originZ + idxZ * size;
		push({ originX - halfX * size, originY, z }, { originX + halfX * size, originY, z }, col);
	}
}

void GridSystem::BuildHoverVertices(const GridParams& params, bool hoverValid, const CellRect& rect, vector<VertexColor>& fill, vector<VertexColor>& border) const
{
	fill.clear();
	border.clear();

	fill.reserve(6);
	border.reserve(8); 

	if (!params.showHover || !hoverValid || rect.Empty()) return;

	_float3 cellMin{};
	_float3 cellMax{};
	{
		const float size = params.cellSize;
		const float originX = params.origin.x;
		const float originY = params.origin.y;
		const float originZ = params.origin.z;

		const float minX = originX + rect.minX * size;
		const float minZ = originZ + rect.minZ * size;
		const float maxX = originX + (rect.maxX + 1) * size;
		const float maxZ = originZ + (rect.maxZ + 1) * size;

		cellMin = { minX, originY, minZ };
		cellMax = { maxX, originY, maxZ };
	}

	// 살짝띄워서 z-fighting 방지
	const float epsilon = 0.001f;
	cellMin.y += epsilon;
	cellMax.y += epsilon;

	const _float3 bottomLeft { cellMin.x, cellMin.y, cellMin.z };
	const _float3 bottomRight{ cellMax.x, cellMin.y, cellMin.z };
	const _float3 topRight   { cellMax.x, cellMin.y, cellMax.z };
	const _float3 topLeft    { cellMin.x, cellMin.y, cellMax.z };

	const _float4 cFill = params.colorHover;
	const _float4 cBorder = params.colorHoverBorder;

	fill.push_back({ bottomLeft,  cFill });
	fill.push_back({ bottomRight, cFill });
	fill.push_back({ topRight,    cFill });

	fill.push_back({ bottomLeft, cFill });
	fill.push_back({ topRight,   cFill });
	fill.push_back({ topLeft,    cFill });

	// Border
	auto edge = [&](_float3 a, _float3 b)
		{
			border.push_back({ a, cBorder });
			border.push_back({ b, cBorder });
		};
	edge(bottomLeft, bottomRight);
	edge(bottomRight, topRight);
	edge(topRight, topLeft);
	edge(topLeft, bottomLeft);
}

CellRect GridSystem::WorldAABBToCellRect(const GridParams& params, const _float3& minWorld, const _float3& maxWorld) const
{
	const float epsilon = 1e-6f;
	_float3 minAdj = minWorld;
	_float3 maxAdj = maxWorld;
	maxAdj.x -= epsilon;
	maxAdj.z -= epsilon;

	const auto worldtocell = [&](const _float3& world) -> GridCoord
		{
			const float numX = (world.x - params.origin.x) / params.cellSize;
			const float numZ = (world.z - params.origin.z) / params.cellSize;
			return { (int)floorf(numX), (int)floorf(numZ) };
		};
	GridCoord coord0 = worldtocell(minAdj);
	GridCoord coord1 = worldtocell(maxAdj);
	CellRect rect;
	rect.minX = min(coord0.x, coord1.x);
	rect.maxX = max(coord0.x, coord1.x);
	rect.minZ = min(coord0.z, coord1.z);
	rect.maxZ = max(coord0.z, coord1.z);
	return rect;
}

CellRect GridSystem::ClampRectToGrid(const GridParams& params, const CellRect& rect) const
{
	const int halfX = params.cellCountX / 2;
	const int halfZ = params.cellCountZ / 2;
	CellRect out = rect;
	out.minX = max(out.minX, -halfX);
	out.maxX = min(out.maxX,  halfX - 1);
	out.minZ = max(out.minZ, -halfZ);
	out.maxZ = min(out.maxZ,  halfZ - 1);
	return out;
}

bool GridSystem::IsRectInsideGrid(const GridParams& params, const CellRect& rect) const
{
	const int halfX = params.cellCountX / 2;
	const int halfZ = params.cellCountZ / 2;
	const bool inside = (rect.minX >= -halfX) && (rect.maxX < halfX) && (rect.minZ >= -halfZ) && (rect.maxZ < halfZ);
	return inside && !rect.Empty();
}

void GridSystem::EnsureDynamicVB(ComPtr<ID3D11Buffer>& buffer, _uint requiredBytes, _uint& capacityBytes) const
{
	GpuUtil::EnsureDynamicBuffer(DEVICE, buffer, requiredBytes, capacityBytes, D3D11_BIND_VERTEX_BUFFER);
}

void GridSystem::UploadVB(ID3D11Buffer* buffer, const void* srcData, _uint bytes) const
{
	GpuUtil::UploadDynamicBuffer(DC, buffer, srcData, bytes);
}

void GridSystem::RenderGui(EntityID id)
{
#ifdef USE_IMGUI
	auto drawOne = [&](Handle handle, GridData& data)
		{
			ImGui::PushID((int)handle.idx);

			if (ImGui::CollapsingHeader("Grid"))
			{
				GridParams param = data.params;

				float cell = param.cellSize;
				if (ImGui::DragFloat("Cell Size", &cell, 0.01f, 1e-6f, 10000.f, "%.3f"))
					SetCellSize(handle, cell);

				int countX = param.cellCountX;
				int countZ = param.cellCountZ;

				if (ImGui::InputInt("Count X", &countX)) SetCellCount(handle, countX, param.cellCountZ);
				if (ImGui::InputInt("Count Z", &countZ)) SetCellCount(handle, param.cellCountX, countZ);

				int majorEvery = param.majorEvery;
				if (ImGui::InputInt("Major Every", &majorEvery)) SetMajorEvery(handle, majorEvery);

				_float3 origin = param.origin;
				if (ImGui::DragFloat3("Origin", &origin.x, 0.1f))
					SetOrigin(handle, origin);

				bool showMinor = param.showMinor;
				bool showMajor = param.showMajor;
				bool showHover = param.showHover;

				bool changed = false;
				changed |= ImGui::Checkbox("Show Minor", &showMinor);
				changed |= ImGui::Checkbox("Show Major", &showMajor);
				changed |= ImGui::Checkbox("Show Hover", &showHover);

				if (changed)
				{
					GridParams newParam = param;
					newParam.showMinor = showMinor;
					newParam.showMajor = showMajor;
					newParam.showHover = showHover;
					SetParams(handle, newParam);
				}

				// Colors
				_float4 cMinor = param.colorMinor;
				_float4 cMajor = param.colorMajor;
				_float4 cHover = param.colorHover;
				_float4 cHoverBorder = param.colorHoverBorder;

				bool colorChanged = false;
				colorChanged |= ImGui::ColorEdit4("Color Minor", &cMinor.x, ImGuiColorEditFlags_Float);
				colorChanged |= ImGui::ColorEdit4("Color Major", &cMajor.x, ImGuiColorEditFlags_Float);
				colorChanged |= ImGui::ColorEdit4("Color Hover", &cHover.x, ImGuiColorEditFlags_Float);
				colorChanged |= ImGui::ColorEdit4("Hover Border", &cHoverBorder.x, ImGuiColorEditFlags_Float);

				if (colorChanged)
				{
					GridParams newParam = param;
					newParam.colorMinor = cMinor;
					newParam.colorMajor = cMajor;
					newParam.colorHover = cHover;
					newParam.colorHoverBorder = cHoverBorder;
					SetParams(handle, newParam);
				}

				// Debug 
				ImGui::Separator();
				ImGui::Text("VB Lines: %u verts, %u bytes cap",
					data.lineVertexCount, data.vbLinesCapacityBytes);
				ImGui::Text("VB Hover Fill: %u verts, %u bytes cap",
					data.hoverFillVertexCount, data.vbHoverFillCapacityBytes);
				ImGui::Text("VB Hover Border: %u verts, %u bytes cap",
					data.hoverBorderVertexCount, data.vbHoverBorderCapacityBytes);

				if (ImGui::Button("Rebuild"))
				{
					data.linesDirty = true;
					data.hoverDirty = true;
					Build(handle);
				}

				ImGui::SameLine();
				if (ImGui::Button("Clear Hover"))
				{
					data.hoverRect = {};
					data.hoverValid = false;
					data.hoverDirty = true;
				}
			}
			ImGui::PopID();
		};

	ForEachOwned(id, [&](Handle handle, GridData& data)
		{
			drawOne(handle, data);
		});
#endif
}