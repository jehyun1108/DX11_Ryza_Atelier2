#pragma once

NS_BEGIN(Client)

class Logo final : public Level
{
public:
	static unique_ptr<Logo> Create();

	virtual HRESULT Init() override;
	virtual void Update(float dt) override;
	virtual void Render() override;

private:
	void LoadResources();
	void AddMaterials();

private:
};

NS_END