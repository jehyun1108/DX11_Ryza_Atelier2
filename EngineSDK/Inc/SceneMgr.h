#pragma once

NS_BEGIN(Engine)

enum class SCENE { MAIN };

class SceneMgr : public Base
{
private:
	SceneMgr() = default;
	virtual ~SceneMgr() = default;

public:
	static SceneMgr* Create();

	HRESULT Init() { return {}; }
	void Update(float dt) {}
	void LateUpdate(float dt) {}

private:
	Scene* scene{};
	SCENE sceneType{};
};

NS_END