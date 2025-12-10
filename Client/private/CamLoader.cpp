#include "pch.h"
#include "CamLoader.h"

void CamLoader::LoadCamResources(SystemRegistry& registry)
{
	auto& camReg = registry.Get<CamRegistry>();
	//camReg.RegisterCamClip(L"../bin/Resources/Camera/Patricia/basicAttack.cam");
	//camReg.RegisterCamClip(L"../bin/Resources/Camera/Patricia/Skill_A1.cam");
	camReg.RegisterCamClip(L"../bin/Resources/Camera/Patricia/Reward_1.cam");
	
	//camReg.RegisterCamClip(L"../bin/Resources/Camera/Ryza/basicAttack.cam");
	camReg.RegisterCamClip(L"../bin/Resources/Camera/Ryza/Reward_1.cam");

	//camReg.RegisterCamClip(L"../bin/Resources/Camera/Klaudia/basicAttack.cam");
	camReg.RegisterCamClip(L"../bin/Resources/Camera/Klaudia/Reward_1.cam");

	camReg.RegisterCamClip(L"../bin/Resources/Camera/Default/Intro_2.cam");
	//camReg.RegisterCamClip(L"../bin/Resources/Camera/Default/Intro.cam");
}