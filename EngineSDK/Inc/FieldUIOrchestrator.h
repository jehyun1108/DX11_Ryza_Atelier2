#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL FieldUIOrchestrator : public ISystem
{
public:
    explicit FieldUIOrchestrator(SystemRegistry& registry) : registry(registry) {}
    void     OnBoot() override;

    void Enter();
    void Tick(float dt);
    void Exit();

private:
    SystemRegistry& registry;
    UIRegistry*     uiRegistry{};
    UISystem*       uiSys{};
    UIAnimSystem*   uiAnimSys{};
};

NS_END