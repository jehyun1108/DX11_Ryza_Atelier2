#pragma once

NS_BEGIN(MapTool)

class MainApp final
{
public:
	static unique_ptr<MainApp> Create();

	HRESULT Init();
	void Update(_float dt);
	HRESULT Render();

private:
	GameInstance& game = GameInstance::GetInstance();
};

NS_END