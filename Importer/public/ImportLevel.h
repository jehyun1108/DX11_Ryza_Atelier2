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
};

NS_END