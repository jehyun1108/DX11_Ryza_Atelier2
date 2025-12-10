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
	void SpawnEntities();

private:
	Loader loader;
};

NS_END