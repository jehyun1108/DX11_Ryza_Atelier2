#pragma once

NS_BEGIN(Engine)

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

	void Clear()
	{
		lights.clear();
		queues.Clear();
		drawColliders = false;
		colliders.clear();
	}
};

NS_END