#pragma once

#include "TextLayoutData.h"

NS_BEGIN(Engine)

class ENGINE_DLL TextLayoutSystem : public ISystem
{
public:
	explicit TextLayoutSystem(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void       BuildTextQuads(const TextLayoutDesc& desc, vector<UIDrawItem>& out) const;
	TextBounds Measure(const TextLayoutDesc& desc) const;

private:
	SystemRegistry& registry;
	FontSystem*     fontSys{};
};

NS_END