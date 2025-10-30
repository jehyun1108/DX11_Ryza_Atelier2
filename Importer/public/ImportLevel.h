#pragma once

NS_BEGIN(Importer)

class ImportLevel final : public Level
{
public:
	static unique_ptr<ImportLevel> Create();

	virtual HRESULT Init() override;
	virtual void Update(float dt) override;
	virtual void Render() override;

private:
	EntityHandles SpawnPatricia();
	EntityHandles SpawnKlaudia();
	EntityHandles SpawnRyza();

	void SetUp();

	void UpdateGrid();
	void DrawSkyBox(bool isNight = true);

private:
	Handle fieldCtrlHandle{};
};

NS_END