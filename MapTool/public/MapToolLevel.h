#pragma once

NS_BEGIN(MapTool)

class MapToolLevel final : public Level
{
public:
	static unique_ptr<MapToolLevel> Create();

	virtual HRESULT Init() override;
	virtual void Update(float dt) override;
	virtual void Render() override;

private:
	Grid* grid = nullptr;
};

NS_END