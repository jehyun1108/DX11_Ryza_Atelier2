#pragma once

NS_BEGIN(Engine)

enum class KEY
{
	UP, DOWN, LEFT, RIGHT, SPACE,
	A, B, C, D, E, F, G, H, I, J, K, L, M,
	N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
	NUM0, NUM1, NUM2, NUM3, NUM4, NUM5, NUM6, NUM7, NUM8, NUM9,
	LSHIFT, RSHIFT, LCTRL, RCTRL,
	TAB, ESC, LBUTTON, RBUTTON,
	END
};

enum class KEY_STATE 
{ 
	NONE, 
	DOWN, 
	PRESSING,
	RELEASE 
};

constexpr size_t KEY_COUNT = static_cast<size_t>(KEY::END);

struct KeyInfo { KEY_STATE state = KEY_STATE::NONE; };

namespace Win32
{
	// lParam�� 30��° ��Ʈ�� 1���� 0���� Ȯ���ϱ� ���� ��.
	// 0�̸�: Ű�� ó������ ���ȴٴ� �ǹ�. 1�̸� Ű�� �� ������ ������ �߻��ϴ� �ݺ� �޼������ �ǹ�.
	constexpr _uint PREV_KEY_STATE_MASK = (1 << 30); 
	// lParam�� 24��° ��Ʈ�� Ȯ���ϱ� ���� ��. ���� Ű�� Ȯ�� Ű ������ �����ϱ� ����.
	constexpr _uint EXTENDED_KEY_FLAG   = (1 << 24);

	// scan code ���� bitshift & mask
	// lParam�� 16~23��° ��Ʈ�� ��ĵ�ڵ� ������ ������ִ�. �� ������ �� ���������� ������������ ���������� 16��Ʈ��ŭ �о������.
	constexpr _uint SCAN_CODE_SHIFT = 16;
	// 16��Ʈ��ŭ �� ������ ���� 8��Ʈ(0~255)�� ����� �������� ��� 0���� ����������. 0xFF �� �������� 11111111 �̹Ƿ�, & ������ ���� ��Ȯ�� 8��Ʈ�� ��ĵ �ڵ� ���� �ɷ����� �ִ�.
	constexpr _uint SCAN_CODE_MASK  = 0xFF;

	constexpr array<KEY, 256> CreateKeyMap()
	{
		array<KEY, 256> map{};

		for (int i{}; i < map.size(); ++i)
			map[i] = KEY::END;

		for (char c = 'A'; c <= 'Z'; ++c)
			map[c] = static_cast<KEY>(static_cast<_uint>(KEY::A) + (c - 'A'));

		for (char c = '0'; c <= '9'; ++c)
			map[c] = static_cast<KEY>(static_cast<_uint>(KEY::NUM0) + (c - '0'));

		for (int i{}; i <= 9; ++i)
			map[VK_NUMPAD0 + i] = static_cast<KEY>(static_cast<_uint>(KEY::NUM0) + i);

		map[VK_UP]       = KEY::UP;
		map[VK_DOWN]     = KEY::DOWN;
		map[VK_LEFT]     = KEY::LEFT;
		map[VK_RIGHT]    = KEY::RIGHT;
		map[VK_SPACE]    = KEY::SPACE;
		map[VK_TAB]      = KEY::TAB;
		map[VK_ESCAPE]   = KEY::ESC;
		map[VK_LBUTTON]  = KEY::LBUTTON;
		map[VK_RBUTTON]  = KEY::RBUTTON;
		map[VK_LSHIFT]   = KEY::LSHIFT;
		map[VK_RSHIFT]   = KEY::RSHIFT;
		map[VK_LCONTROL] = KEY::LCTRL;
		map[VK_RCONTROL] = KEY::RCTRL;

		return map;
	}
	static constexpr array<KEY, 256> keyMap = CreateKeyMap();

	// lParam�� �м��Ͽ� ��/�� Ű ���� ����
	static KEY MapVirtualToKey(WPARAM wParam, LPARAM lParam)
	{
		const _uint vk = static_cast<_uint>(wParam);

		if (vk == VK_SHIFT || vk == VK_CONTROL || vk == VK_MENU) 
		{
			const _uint scanCode   = (lParam >> Win32::SCAN_CODE_SHIFT) & Win32::SCAN_CODE_MASK;
			const bool  isExtended = (lParam & Win32::EXTENDED_KEY_FLAG) != 0;

			if (vk == VK_SHIFT)
			{
				const _uint lShiftSc = MapVirtualKeyW(VK_LSHIFT, MAPVK_VK_TO_VSC);
				return (scanCode == lShiftSc) ? KEY::LSHIFT : KEY::RSHIFT;
			}
			if (vk == VK_CONTROL)
				return isExtended ? KEY::RCTRL : KEY::LCTRL;

			if (vk == VK_MENU)
				return KEY::END;
		}

		if (vk < 256)
			return keyMap[vk];

		return KEY::END;
	}
}

NS_END