#pragma once

NS_BEGIN(Engine)

namespace TypeID
{
	// ID ī���͸� ���� ���� �Լ�
	inline size_t next_id()
	{
		static size_t counter = 0;
		return counter++;
	}

	// Ư�� Ÿ�� T�� ���� ���� ID �� ��ȯ�ϴ� ���ø� �Լ�
	template<typename T>
	size_t get()
	{
		static const size_t id = next_id();
		return id;
	}
}

NS_END