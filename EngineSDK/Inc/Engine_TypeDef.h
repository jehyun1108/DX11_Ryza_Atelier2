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
		      
    // 저장용 데이터 
	typedef    XMFLOAT2       _float2;
	typedef    XMFLOAT3       _float3;
	typedef    XMFLOAT4       _float4;
	typedef    XMFLOAT4X4     _float4x4;
		      
	// SIMD 연산을 위한 데이터 선언(병렬적 연산)
	typedef    XMVECTOR       _vec;
	typedef    FXMVECTOR      _fvec;
	typedef    GXMVECTOR      _gvec;
	typedef    HXMVECTOR      _hvec;
	typedef    CXMVECTOR      _cvec;
	
	typedef    XMMATRIX       _mat;
	typedef    FXMMATRIX      _fmat;
	typedef    CXMMATRIX      _cmat;

	// XMVECTOR 는 SIMD 레지스터에 데이터를 올려놓고 연산하기 위해 특별히 설계된 타입이다.
	// CPU가 최고의 성능을 내려면, 메모리에 있던 데이터를 SIMD 레지스터로 불러와서(Load) 연산하고,
	// 결과를 다시 메모리로 "저장(Store)" 하는 과정이 반드시 필요하다.
}