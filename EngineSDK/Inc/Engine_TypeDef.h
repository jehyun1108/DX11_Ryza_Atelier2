#pragma once

namespace Engine
{
	typedef    bool           _bool;				   
	typedef    signed char    _byte;				   
	typedef    unsigned char  _ubyte;				   
	typedef    char           _char;				   
	typedef    wchar_t        _tchar;
	typedef    wstring        _wstring;				   
	typedef    signed short   _short;
	typedef    unsigned short _ushort;
	typedef    signed int     _int;
	typedef    unsigned int   _uint;
	typedef    signed long    _long;
	typedef    unsigned long  _ulong;
	typedef    float          _float;
	typedef    double         _double;
		      
    // ����� ������ 
	typedef    XMFLOAT2       _float2;
	typedef    XMFLOAT3       _float3;
	typedef    XMFLOAT4       _float4;
	typedef    XMFLOAT4X4     _float4x4;
		      
	// SIMD ������ ���� ������ ����(������ ����)
	typedef    XMVECTOR       _vec;
	typedef    FXMVECTOR      _fvec;
	typedef    GXMVECTOR      _gvec;
	typedef    HXMVECTOR      _hvec;
	typedef    CXMVECTOR      _cvec;
	
	typedef    XMMATRIX       _mat;
	typedef    FXMMATRIX      _fmat;
	typedef    CXMMATRIX      _cmat;

	// XMVECTOR �� SIMD �������Ϳ� �����͸� �÷����� �����ϱ� ���� Ư���� ����� Ÿ���̴�.
	// CPU�� �ְ��� ������ ������, �޸𸮿� �ִ� �����͸� SIMD �������ͷ� �ҷ��ͼ�(Load) �����ϰ�,
	// ����� �ٽ� �޸𸮷� "����(Store)" �ϴ� ������ �ݵ�� �ʿ��ϴ�.
}