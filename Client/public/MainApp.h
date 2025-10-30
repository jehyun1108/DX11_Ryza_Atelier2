#pragma once

NS_BEGIN(Client)

class MainApp final
{
public:
	MainApp() = default;

public:
	static unique_ptr<MainApp> Create();

	HRESULT Init();
	void Update(_float dt);
	HRESULT Render();

private:
	GameInstance& game = GameInstance::GetInstance();

	ID3D11Device* device{};
	ID3D11DeviceContext* context{};
};

NS_END