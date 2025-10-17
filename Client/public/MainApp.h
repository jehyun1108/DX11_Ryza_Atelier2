#pragma once

NS_BEGIN(Client)

class MainApp final
{
public:
	MainApp() :game(GameInstance::GetInstance()) {}

public:
	static unique_ptr<MainApp> Create();

	HRESULT Init();
	void Update(_float dt);
	HRESULT Render();

private:
	void StartLevel(LEVEL startID);

private:
	GameInstance& game;

	ID3D11Device* device{};
	ID3D11DeviceContext* context{};
};

NS_END