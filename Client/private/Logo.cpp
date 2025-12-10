#include "pch.h"
#include "Logo.h"
#include "Loading.h"

unique_ptr<Logo> Logo::Create()
{
	auto instance = make_unique<Logo>();
	if (FAILED(instance->Init()))
		return nullptr;
	 
	return instance;
}

HRESULT Logo::Init()
{
	return S_OK;
}

void Logo::Update(float dt)
{			
}

void Logo::Render()
{
}

