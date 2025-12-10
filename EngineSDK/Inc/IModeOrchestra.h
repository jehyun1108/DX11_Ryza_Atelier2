#pragma once

NS_BEGIN(Engine)

enum class GameMode { None, Field, Battle, Menu };

struct ENGINE_DLL IModeOrchestrator
{
	virtual ~IModeOrchestrator() {}
	virtual void Enter() = 0;
	virtual void Update(float dt) = 0;
	virtual void Exit() = 0;
};

NS_END