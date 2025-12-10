#pragma once

NS_BEGIN(Client)

class MainApp final
{
public:
	static unique_ptr<MainApp> Create();

	HRESULT Init();
	void Update(_float dt);
	HRESULT Render();

	void LoadLoadingResources();

private:
	GameInstance& game = GameInstance::GetInstance();

	ID3D11Device* device{};
	ID3D11DeviceContext* context{};
};

NS_END