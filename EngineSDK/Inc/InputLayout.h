#pragma once

NS_BEGIN(Engine)

enum class VtxAttribute : _uint
{
	None            = 0,
	Pos             = 1 << 0,
	Color           = 1 << 1,
	TexCoord        = 1 << 2,
	Normal          = 1 << 3,
	Tangent         = 1 << 4, 
	World           = 1 << 5,
    BlendIndices    = 1 << 6,
    BlendWeights    = 1 << 7,
};
  
inline VtxAttribute operator|(VtxAttribute lhs, VtxAttribute rhs)
{
	return static_cast<VtxAttribute>(static_cast<_uint>(lhs) | static_cast<_uint>(rhs));
}
inline bool operator&(VtxAttribute lhs, VtxAttribute rhs)
{
	return (static_cast<_uint>(lhs) & static_cast<_uint>(rhs)) != 0;
}

struct VtxElement
{
	VtxAttribute attribute;
	_uint sementicIdx = {};
	_uint inputSlot = {};
	// �ν��Ͻ��� ���� ����, 0�̸� per-vertex, 1 �̻��̸� per-instance.
	_uint instanceStepRate = {};
};

class InputLayout
{
public:
    // ü�̴�(Chaining)�� ���� �ڱ� �ڽ� ���۷��� ��ȯ
    InputLayout& Add(const VtxElement& element)
    {
        elements.push_back(element);
        return *this;
    }

    InputLayout& Add(VtxAttribute attribute)
    {
        elements.push_back({ attribute });
        return *this;
    }

    vector<D3D11_INPUT_ELEMENT_DESC> Build() const
    {
        vector<D3D11_INPUT_ELEMENT_DESC> descs;
        // �� InputSlot ���� �޸� �������� �ڵ����� ����ϱ� ���� map
        map<_uint, _uint> offsets;

        for (const auto& elem : elements)
        {
            // ���� ���(matrix)�� 4���� float4�� �̷���� �־� ���� ó��
            if (elem.attribute & VtxAttribute::World)
            {
                // D3D11_INPUT_ELEMENT_DESC 4���� �߰�
                for (_uint i{}; i < 4; ++i)
                {
                    descs.push_back({
                        "WORLD", i, DXGI_FORMAT_R32G32B32A32_FLOAT, elem.inputSlot,
                        offsets[elem.inputSlot], D3D11_INPUT_PER_INSTANCE_DATA, elem.instanceStepRate
                        });
                    offsets[elem.inputSlot] += sizeof(_float4);
                }
            }
            else
            {
                // �� �� �Ϲ� �Ӽ���
                D3D11_INPUT_ELEMENT_DESC desc = {};
                desc.InputSlot = elem.inputSlot;
                desc.SemanticIndex = elem.sementicIdx;
                desc.InputSlotClass = (elem.instanceStepRate > 0) ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
                desc.InstanceDataStepRate = elem.instanceStepRate;
                desc.AlignedByteOffset = offsets[elem.inputSlot];

                switch (elem.attribute)
                {
                case VtxAttribute::Pos:
                    desc.SemanticName = "POSITION";
                    desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
                    offsets[elem.inputSlot] += sizeof(_float3);
                    break;
                case VtxAttribute::Color:
                    desc.SemanticName = "COLOR";
                    desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
                    offsets[elem.inputSlot] += sizeof(_float4);
                    break;
                case VtxAttribute::TexCoord:
                    desc.SemanticName = "TEXCOORD";
                    desc.Format = DXGI_FORMAT_R32G32_FLOAT;
                    offsets[elem.inputSlot] += sizeof(_float2);
                    break;
                case VtxAttribute::Normal:
                    desc.SemanticName = "NORMAL";
                    desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
                    offsets[elem.inputSlot] += sizeof(_float3);
                    break;
                case VtxAttribute::Tangent:
                    desc.SemanticName = "TANGENT";
                    desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
                    offsets[elem.inputSlot] += sizeof(_float3);
                    break;
                case VtxAttribute::BlendIndices:
                    desc.SemanticName = "BLENDINDICES";
                    desc.Format = DXGI_FORMAT_R32G32B32A32_UINT; 
                    offsets[elem.inputSlot] += sizeof(_uint) * 4;
                    break;
                case VtxAttribute::BlendWeights:
                    desc.SemanticName = "BLENDWEIGHT";
                    desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; 
                    offsets[elem.inputSlot] += sizeof(float) * 4;
                    break;
                }
                descs.push_back(desc);
            }
        }

        assert(!descs.empty() && "Failed to create any vertex element desc");
        return descs;
    }

private:
    vector<VtxElement> elements;
};
NS_END