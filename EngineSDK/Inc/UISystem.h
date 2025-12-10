#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL UISystem : public ISystem
{
public:
	explicit UISystem(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void Tick(float dt) {}
	void ExtractUIProxies(UISnapShot& out);

	pair<float, float> ToNorm(UIAnchor anchor);
	pair<float, float> ToNorm(UIPivot pivot);

	void SetText(const wstring& key, float x, float y, const wstring& text);
	void SetText(const wstring& key, const _float2& pos, const wstring& text);
	void SetText(const wstring& key, const wstring& text);

	void      SetActiveContext(UIContext ctx) { activeContext = ctx; }
	UIContext GetActiveContext() const        { return activeContext; }

private:
	UIContext               activeContext = UIContext::Loading;

private:
	SystemRegistry&         registry;
	AssetSystem*            assets{};
	UIRegistry*             uiRegistry{};
	UIAnimSystem*           uiAnimSys{};
	GameModeDirectorSystem* director{};
	TextLayoutSystem*       textLayoutSys{};
};

NS_END