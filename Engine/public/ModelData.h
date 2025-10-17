#pragma once

NS_BEGIN(Engine)
class Model;

struct ENGINE_DLL ModelData
{
	Handle transform{};
	Handle animator{};
	shared_ptr<Model> model{};
	bool   enabled = true;
};

NS_END