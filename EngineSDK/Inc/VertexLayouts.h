#pragma once

NS_BEGIN(Engine)

enum class VtxAttribute : _uint
{
	None     = 0,
	Pos      = 1 << 0,
	Color    = 1 << 1,
	TexCoord = 1 << 2,
	Normal   = 1 << 3,
	Tangent  = 1 << 4, 
};
  
inline VtxAttribute operator|(VtxAttribute lhs, VtxAttribute rhs)
{
	return static_cast<VtxAttribute>(static_cast<_uint>(lhs) | static_cast<_uint>(rhs));
}
inline bool operator&(VtxAttribute lhs, VtxAttribute rhs)
{
	return (static_cast<_uint>(lhs) & static_cast<_uint>(rhs)) != 0;
}

struct Vtx
{
	_float3 pos = {};
	_float4 color = {};
	_float2 uv = {};
	_float3 normal = {};
	_float3 tangent = {};

	// 각 속성에 대한 Descriptor 상수로 미리 정의하여 중복 제거
	static const D3D11_INPUT_ELEMENT_DESC PosDesc;
	static const D3D11_INPUT_ELEMENT_DESC ColorDesc;
	static const D3D11_INPUT_ELEMENT_DESC UVDesc;
	static const D3D11_INPUT_ELEMENT_DESC NormalDesc;
	static const D3D11_INPUT_ELEMENT_DESC TangentDesc;

	static vector<D3D11_INPUT_ELEMENT_DESC> GetLayoutDesc(VtxAttribute attributes)
	{
		vector<D3D11_INPUT_ELEMENT_DESC> descs;

		if (attributes & VtxAttribute::Pos)
			descs.push_back(PosDesc);

		if (attributes & VtxAttribute::Color)
			descs.push_back(ColorDesc);

		if (attributes & VtxAttribute::TexCoord)
			descs.push_back(UVDesc);

		if (attributes & VtxAttribute::Normal)
			descs.push_back(NormalDesc);

		if (attributes & VtxAttribute::Tangent)
			descs.push_back(TangentDesc);

		static_assert((1 << 0) == static_cast<_uint>(VtxAttribute::Pos), "Position flag must be the first bit");
		assert(attributes & VtxAttribute::Pos && "Vertex Layout must include a position attributes.");
		assert(!descs.empty() && "Failed to create any vertex element desc");

		return descs;
	}
};

inline constexpr D3D11_INPUT_ELEMENT_DESC Vtx::PosDesc =     { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vtx, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 };
inline constexpr D3D11_INPUT_ELEMENT_DESC Vtx::ColorDesc =   { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vtx, color), D3D11_INPUT_PER_VERTEX_DATA, 0 };
inline constexpr D3D11_INPUT_ELEMENT_DESC Vtx::UVDesc =      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vtx, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 };
inline constexpr D3D11_INPUT_ELEMENT_DESC Vtx::NormalDesc =  { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vtx, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 };
inline constexpr D3D11_INPUT_ELEMENT_DESC Vtx::TangentDesc = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vtx, tangent), D3D11_INPUT_PER_VERTEX_DATA, 0 };

NS_END

// D3D11_INPUT_ELEMENT_DESC 
// 
// LPCSTR SementicName: 셰이더 입력의 시멤틱 이름 (예: "POSITION" , "TEXCOORD" )
// UINT   SementicIdx : 같은 시멘틱 이름을 구분하기 위한 인덱스 
// DXGI_FORMAT: 데이터의 포맷 (예: DXGI_FORMAT_R32G32B32_FLOAT)
// UINT   InputSlot: 이 데이터를 가져올 정점 버퍼의 슬롯 번호.
// UINT   AlignedByteOffset: 정점 시작부터 이 데이터까지의 오프셋.
// D3D11_INPUT_CLASSIFICATION InputSlotClass: 정점 단위 데이터인지, 인스턴스 단위 데이터인지
// UINT InstanceDataStepRate: 인스턴스 단위 데이터일 경우, 다음 데이터를 읽을 간격

