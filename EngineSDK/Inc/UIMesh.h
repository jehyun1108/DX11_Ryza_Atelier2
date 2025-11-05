#pragma once

NS_BEGIN(Engine)
struct UICB;
class CBuffer;

class ENGINE_DLL UIMesh
{
public:
    HRESULT Create(ID3D11Device* device, size_t maxQuads = 2048);
    void    Destroy();

    void Bind(ID3D11DeviceContext* context, Shader& uiShader, CBuffer& uiCBuffer, const UICB& uiCB);
    void Draw(ID3D11DeviceContext* context, const vector<UIDrawItem>& items, ResolveTexture resolveTexture);

private:
    size_t FillVB(ID3D11DeviceContext* context, const vector<const UIDrawItem*>& sorted,  size_t quadCount);
    void   IssueBatches(ID3D11DeviceContext* context,  const vector<const UIDrawItem*>& sorted, ResolveTexture resolveTexture,  size_t quadCount);

private:
    ComPtr<ID3D11Buffer> vbDynamic;
    ComPtr<ID3D11Buffer> ibStatic;
    size_t               maxQuads = 0;
};

NS_END