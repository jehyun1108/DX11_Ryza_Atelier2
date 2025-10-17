#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL LightMgr final 
{
public:
	static unique_ptr<LightMgr> Create();

	void Init();
	
	const vector<Light*>& GetLights() const { return lights; }

private:
	vector<Light*> lights;
};

NS_END