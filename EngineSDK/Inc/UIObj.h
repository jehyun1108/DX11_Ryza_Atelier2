#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL UIObj : public Obj
{
protected:
	UIObj() = default;
	virtual ~UIObj() = default;

public:
	virtual void Init(const any& arg) override;

	void UpdateTransform();

protected:
	UIDesc uiDesc;

	float viewportW = 0.f;
	float viewportH = 0.f;
};

NS_END