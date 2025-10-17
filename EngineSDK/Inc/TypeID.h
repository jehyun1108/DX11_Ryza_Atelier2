#pragma once

NS_BEGIN(Engine)

namespace TypeID
{
	// ID 카운터를 위한 내부 함수
	inline size_t next_id()
	{
		static size_t counter = 0;
		return counter++;
	}

	// 특정 타입 T에 대한 고유 ID 를 반환하는 템플릿 함수
	template<typename T>
	size_t get()
	{
		static const size_t id = next_id();
		return id;
	}
}

NS_END