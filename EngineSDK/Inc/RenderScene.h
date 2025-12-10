#pragma once

NS_BEGIN(Engine)
struct UIDrawItem;

struct UISnapShot
{
	vector<UIDrawItem> drawItems;
	void Clear() { drawItems.clear(); }
};
struct RenderScene
{
	RenderQueues queues{};
	
	CameraProxy           cam{};
	vector<LightProxy>    lights{};
	vector<ColliderProxy> colliders;
	SkyboxProxy           skybox{};
	UISnapShot            ui{};
	CameraProxy           minimapCam{};
	ParticleSnapshot      particles;
	TrailSnapshot         trails;

	bool drawColliders = false;
	bool minimapEnabled = false;

	void Clear()
	{
		skybox     = {};
		minimapCam = {};
		lights.clear();
		colliders.clear();
		queues.Clear();
		ui.Clear();
		particles.Clear();
		trails.Clear();
		drawColliders = false;
		minimapEnabled = false;
	}
};

NS_END