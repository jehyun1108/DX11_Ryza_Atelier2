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
	// ºÒº¯ SnapShot
	CameraProxy        cam{};
	vector<LightProxy> lights{};

	// DrawQueue
	RenderQueues queues{};

	// Debug
	bool drawColliders = false;
	vector<ColliderProxy> colliders;

	// Skybox
	SkyboxProxy skybox{};

	// UI
	UISnapShot ui{};

	void Clear()
	{
		lights.clear();
		queues.Clear();
		drawColliders = false;
		colliders.clear();
		skybox        = {};
		ui.Clear();
	}
};

NS_END