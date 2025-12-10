#pragma once

NS_BEGIN(Engine)
class FieldUIOrchestrator;

class ENGINE_DLL FieldOrchestraSystem : public IModeOrchestrator, public ISystem
{
public:
	explicit FieldOrchestraSystem(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void Enter() override;
	void Update(float dt) override;
	void Exit() override;

private:
	void BuildEnemiesAroundHit(const _float3& center, EntityID primaryTarget, const vector<EntityID>& allEnemies, BattleEnemies& outEnemies);
	void CollectFieldEnemies(vector<EntityID>& out);

private:
	SystemRegistry&         registry;
	FieldUIOrchestrator*    uiOrchestrator{};
	InputService*           input{};
	FieldControllerSystem*  fieldCtrlSys{};
	FieldAnimSystem*        fieldAnimSys{};
	BattleSessionSystem*    sessionSys{};
	CharacterDataSystem*    dataSys{};
	GameModeDirectorSystem* director{};
	ScreenDistortionSystem* distortionSys{};
	CollisionSystem*        collisionSys{};
	TransformSystem*        tfSys{};
	LayerSystem*            layerSys{};
};

NS_END