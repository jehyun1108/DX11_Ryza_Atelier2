#pragma once

NS_BEGIN(Engine)

struct ENGINE_DLL EntityHandles
{
	EntityID entity{};

	Handle tf{};
	Handle cam{};
	Handle freeCam{};
	Handle light{};
	Handle layer{};
	Handle animator{};
	Handle model{};
	Handle face{};
	Handle mouth{};
	Handle socket{};
	Handle grid{};
	Handle picking{};
	Handle collision{};
	Handle skybox{};
	Handle faceAnim{};
	Handle orbitCam{};

	explicit operator bool() const { return entity != invalidEntity; }
};

NS_END