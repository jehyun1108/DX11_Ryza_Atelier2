#pragma once

NS_BEGIN(MapTool)

enum class MAPTOOL { SELECT, PLACE, END };

class MapToolPanel final : public GuiPanel
{
public:
	MapToolPanel() : GuiPanel("MapTool") {}

public:
	virtual void Draw() override;

	virtual void Update(float dt) override;
	void UpdateTool();

	optional<filesystem::path> GetSelectedModelPath() const;
	void RefreshModelList();

	void SetGrid(Grid* grid) { this->grid = grid; }

private:
	MAPTOOL curState = MAPTOOL::SELECT;
	vector<filesystem::path> modelFiles;
	int selectedModelIdx = -1;

	Obj* pickedObj{};
	class MapObj* previewObj{};
	_uint objCounter = 0;

	Grid* grid{};
	float placeYOffset = 0.01f;
};

NS_END