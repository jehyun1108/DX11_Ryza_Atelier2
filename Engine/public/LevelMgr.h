#pragma once

NS_BEGIN(Engine)

class LevelMgr final
{
public:
	static unique_ptr<LevelMgr> Create() { return make_unique<LevelMgr>(); }

	void ChangeLevel(_uint _levelID, unique_ptr<Level> newLevel);
	void Update(float dt);
	void Render();

	_uint GetCurLevelID() const { return levelID; }

private:
	_uint             levelID{};
	unique_ptr<Level> curLevel{};
	GameInstance&     gameInstance = GameInstance::GetInstance();
};

NS_END