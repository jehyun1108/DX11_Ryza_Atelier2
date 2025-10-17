#pragma once

namespace Engine
{
	template<typename T>
	void Safe_Delete(T& p)
	{
		if (p)
		{
			delete p;
			p = nullptr;
		}
	}

	template<typename T>
	void Safe_Delete_Array(T& p)
	{
		if (p)
		{
			delete[] p;
			p = nullptr;
		}
	}

	template<typename T>
	_uint Safe_AddRef(T& inst)
	{
		_uint refCnt{};

		if (inst)
			refCnt = inst->AddRef();

		return refCnt;
	}

	template<typename T>
	_uint Safe_Release(T& inst)
	{
		_uint refCnt{};

		if (inst)
		{
			refCnt = inst->Release();

			if (refCnt == 0)
				inst = nullptr;
		}

		return refCnt;
	}
}