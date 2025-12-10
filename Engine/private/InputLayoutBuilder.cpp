#include "Enginepch.h"

InputLayoutBuilder& InputLayoutBuilder::Add(VtxAttribute attr, _uint semanticIdx, _uint slot, _uint stepRate)
{
	assert(slot < maxSlots && "slot out of range");
	if (attr == VtxAttribute::World)
	{
		AddWorldMat(slot, stepRate > 0 ? stepRate : 1);
		return *this;
	}

	D3D11_INPUT_ELEMENT_DESC desc{};
	FillElement(attr, semanticIdx, slot, stepRate, desc);

	desc.AlignedByteOffset = offsets[slot];
	elements.push_back(desc);
	offsets[slot] += SizeOf(attr);
	return *this;;
}

InputLayoutBuilder& InputLayoutBuilder::AddWorldMat(_uint slot, _uint stepRate)
{
	static constexpr const char* SEMANTIC = "WORLD";
	for (_uint i = 0; i < 4; ++i)
	{
		D3D11_INPUT_ELEMENT_DESC desc{};
		desc.SemanticName         = SEMANTIC;
		desc.SemanticIndex        = i;
		desc.Format               = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.InputSlot            = slot;
		desc.AlignedByteOffset    = offsets[slot];
		desc.InputSlotClass       = D3D11_INPUT_PER_INSTANCE_DATA;
		desc.InstanceDataStepRate = stepRate;
		elements.push_back(desc);
		offsets[slot] += sizeof(_float4);
	}
	return *this;
}

vector<D3D11_INPUT_ELEMENT_DESC> InputLayoutBuilder::Build() const
{
	return elements;
}

void InputLayoutBuilder::Clear()
{
	elements.clear();
	offsets.fill(0);
}

InputLayoutBuilder InputLayoutBuilder::MakePNUTan()
{
	InputLayoutBuilder builder;
	builder.Add(VtxAttribute::Pos)
		   .Add(VtxAttribute::Normal)
		   .Add(VtxAttribute::TexCoord, 0)
		   .Add(VtxAttribute::Tangent);
	return builder;
}

InputLayoutBuilder InputLayoutBuilder::MakePNUTanSkin()
{
	InputLayoutBuilder builder;
	builder.Add(VtxAttribute::Pos)
		   .Add(VtxAttribute::Normal)
		   .Add(VtxAttribute::TexCoord, 0)
		   .Add(VtxAttribute::Tangent)
		   .Add(VtxAttribute::BlendIndices)
		   .Add(VtxAttribute::BlendWeights);
	return builder;
}

InputLayoutBuilder InputLayoutBuilder::MakeInstancedPNUTan()
{
	InputLayoutBuilder builder = MakePNUTan();
	builder.AddWorldMat(1, 1);
	return builder;
}

InputLayoutBuilder InputLayoutBuilder::MakePC()
{
	InputLayoutBuilder builder;
	builder.Add(VtxAttribute::Pos)
		   .Add(VtxAttribute::Color);
	return builder;
}

InputLayoutBuilder InputLayoutBuilder::MakePU()
{
	InputLayoutBuilder builder;
	builder.Add(VtxAttribute::Pos)
		   .Add(VtxAttribute::TexCoord);
	return builder;
}

InputLayoutBuilder InputLayoutBuilder::MakeUI()
{
	InputLayoutBuilder builder;
	builder.Add(VtxAttribute::PosUI)                // POSITION  (posX,posY)
		.Add(VtxAttribute::TexCoord, 0)         // TEXCOORD0 (uv)
		.Add(VtxAttribute::TexCoord, 1)         // TEXCOORD1 (fill)
		.Add(VtxAttribute::TexCoord1, 2)         // TEXCOORD2 (mode)
		.Add(VtxAttribute::TexCoord1, 3)         // TEXCOORD3 (alpha)
		.Add(VtxAttribute::TexCoord1, 4)         // TEXCOORD4 (maskType)
		.Add(VtxAttribute::Color);
	return builder;
}

InputLayoutBuilder InputLayoutBuilder::MakePSkin()
{
	InputLayoutBuilder builder;
	builder.Add(VtxAttribute::Pos)
		   .Add(VtxAttribute::BlendIndices)
		   .Add(VtxAttribute::BlendWeights);
	return builder;
}

InputLayoutBuilder InputLayoutBuilder::MakeP()
{
	InputLayoutBuilder builder;
	builder.Add(VtxAttribute::Pos);
	return builder;
}

InputLayoutBuilder InputLayoutBuilder::MakeParticle()
{
	InputLayoutBuilder builder;

	// slot 0 : 쿼드 버텍스 (per-vertex)
	builder.Add(VtxAttribute::Pos, 0, 0, 0);       // POSITION0 (float3)
	builder.Add(VtxAttribute::TexCoord, 0, 0, 0);  // TEXCOORD0 (float2, uv)
	// slot 1 : 인스턴스 데이터 (per-instance, stepRate=1)
	builder.Add(VtxAttribute::Pos, 1, 1, 1);       // POSITION1 (float3, center)
	builder.Add(VtxAttribute::TexCoord1, 1, 1, 1); // TEXCOORD1 (float,  size)
	builder.Add(VtxAttribute::Color, 0, 1, 1);     // COLOR0    (float4, color)
	builder.Add(VtxAttribute::TexCoord1, 2, 1, 1); // TEXCOORD2 (float,  rotRad)

	builder.Add(VtxAttribute::TexCoord, 3, 1, 1); // TEXCOORD3 (float2, uvMin)
	builder.Add(VtxAttribute::TexCoord, 4, 1, 1); // TEXCOORD4 (float2, uvMax)
	return builder;
}

InputLayoutBuilder InputLayoutBuilder::MakeTrail()
{
	InputLayoutBuilder builder;
	builder.Add(VtxAttribute::Pos)
		   .Add(VtxAttribute::Color)
		   .Add(VtxAttribute::TexCoord);
	return builder;
}

void InputLayoutBuilder::FillElement(VtxAttribute attr, _uint semanticIdx, _uint slot, _uint stepRate, D3D11_INPUT_ELEMENT_DESC& out)
{
	out.InputSlot            = slot;
	out.SemanticIndex        = semanticIdx;
	out.InputSlotClass       = (stepRate > 0) ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
	out.InstanceDataStepRate = stepRate;

	switch (attr)
	{
	case VtxAttribute::Pos:
		out.SemanticName = "POSITION";
		out.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		break;
	case VtxAttribute::PosUI: 
		out.SemanticName = "POSITION";
		out.Format = DXGI_FORMAT_R32G32_FLOAT; 
		break; 
	case VtxAttribute::Color:
		out.SemanticName = "COLOR";
		out.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		break;
	case VtxAttribute::TexCoord:
		out.SemanticName = "TEXCOORD";
		out.Format = DXGI_FORMAT_R32G32_FLOAT;
		break;
	case VtxAttribute::Normal:
		out.SemanticName = "NORMAL";
		out.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		break;
	case VtxAttribute::Tangent:
		out.SemanticName = "TANGENT";
		out.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		break;
	case VtxAttribute::BlendIndices:
		out.SemanticName = "BLENDINDICES";
		out.Format =  DXGI_FORMAT_R32G32B32A32_UINT;
		break;
	case VtxAttribute::BlendWeights:
		out.SemanticName = "BLENDWEIGHT";
		out.Format  = DXGI_FORMAT_R32G32B32A32_FLOAT;
		break;
	case VtxAttribute::TexCoord1:              
		out.SemanticName = "TEXCOORD";
		out.Format = DXGI_FORMAT_R32_FLOAT;
		break;
	default:
		break;
	}
}

_uint InputLayoutBuilder::SizeOf(VtxAttribute attr)
{
	switch (attr)
	{
	case VtxAttribute::Pos:          return 12; 
	case VtxAttribute::PosUI:        return 8;
	case VtxAttribute::Color:        return 16;
	case VtxAttribute::TexCoord:     return 8; 
	case VtxAttribute::TexCoord1:    return 4;
	case VtxAttribute::Normal:       return 12; 
	case VtxAttribute::Tangent:      return 16; 
	case VtxAttribute::BlendIndices: return 16;  
	case VtxAttribute::BlendWeights: return 16;  
	default:                         return 0;
	}
}