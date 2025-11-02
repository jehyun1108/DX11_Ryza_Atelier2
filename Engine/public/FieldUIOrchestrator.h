#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL FieldUIOrchestrator
{
public:
    FieldUIOrchestrator(SystemRegistry& registry, UIRegistry& uiRegistry, UISystem& uiSys, UIAnimSystem& uiAnimSys) 
        : registry(registry), uiRegistry(uiRegistry),  uiSys(uiSys), uiAnimSys(uiAnimSys) {}

    void Enter();
    void Tick(float dt);
    void Exit();

private:
    SystemRegistry& registry;
    UIRegistry&     uiRegistry;
    UISystem&       uiSys;
    UIAnimSystem&   uiAnimSys;
};

NS_END