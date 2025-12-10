#pragma once

NS_BEGIN(Client)

class Central final : public Level
{
public:
	static unique_ptr<Central> Create();

	virtual HRESULT Init() override;
	virtual void Update(float dt) override;
	virtual void Render() override;

private:
	Handle fieldCtrlHandle{};
};

NS_END