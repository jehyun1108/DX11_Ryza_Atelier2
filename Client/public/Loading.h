#pragma once

#include "Loader.h"

NS_BEGIN(Engine)

class Loading final : public Level
{
public:
	static unique_ptr<Loading> Create();

	virtual HRESULT Init() override;
	virtual void Update(float dt) override;
	virtual void Render() override;

private:
	void LoadResources();

private:
	Loader loader;
	LEVEL  nextLevel = LEVEL::CENTRAL;
};

NS_END