#include "pch.h"
#include "Central.h"
#include "UILoader.h"
#include "CharacterUILoader.h"
#include "WorldSerializer.h"

unique_ptr<Central> Central::Create()
{
	auto instance = make_unique<Central>();
	if (FAILED(instance->Init()))
		return nullptr;
	return instance;
}

HRESULT Central::Init()
{
	return S_OK;
}

void Central::Update(float dt)
{
}

void Central::Render()
{

}