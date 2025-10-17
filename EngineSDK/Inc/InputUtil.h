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
	// lParam의 30번째 비트가 1인지 0인지 확인하기 위한 값.
	// 0이면: 키가 처음으로 눌렸다는 의미. 1이면 키를 꾹 느르고 있을때 발생하는 반복 메세지라는 의미.
	constexpr _uint PREV_KEY_STATE_MASK = (1 << 30); 
	// lParam의 24번째 비트를 확인하기 위한 값. 눌린 키가 확장 키 인지를 구분하기 위해.
	constexpr _uint EXTENDED_KEY_FLAG   = (1 << 24);

	// scan code 관련 bitshift & mask
	// lParam의 16~23번째 비트에 스캔코드 정보가 담겨져있다. 이 정보를 맨 오른쪽으로 가져오기위해 오른쪽으로 16비트만큼 밀어버린다.
	constexpr _uint SCAN_CODE_SHIFT = 16;
	// 16비트만큼 민 값에서 하위 8비트(0~255)만 남기고 나머지는 모드 0으로 만들어버린다. 0xFF 는 이진수로 11111111 이므로, & 연산을 통해 정확히 8비트의 스캔 코드 값만 걸러낼수 있다.
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

	// lParam을 분석하여 좌/우 키 등을 구분
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