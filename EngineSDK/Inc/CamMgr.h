#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL CamMgr final
{
public:
	static unique_ptr<CamMgr> Create() { return make_unique<CamMgr>(); }

	void RegisterCam(Camera* cam);
	void UnRegisterCam(Camera* cam);

	void SetMainCam(Camera* cam);
	Camera* GetMainCam() const { return mainCam; }

private:
	vector<Camera*> cams;
	Camera* mainCam{};
};

NS_END