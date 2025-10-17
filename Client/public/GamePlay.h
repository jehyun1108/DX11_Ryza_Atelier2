#pragma once

NS_BEGIN(Client)

class GamePlay final : public Level
{
public:
	virtual HRESULT Init() override;
	virtual void Update(float dt) override;
	virtual void Render() override;
};

NS_END