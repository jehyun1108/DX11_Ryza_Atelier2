#include "pch.h"
#include "Loader.h"

Loader::~Loader()
{
	if (_thread.joinable())
		_thread.join();
}

HRESULT Loader::Init(LEVEL nextLevelID)
{
	this->nextLevelID = nextLevelID;

	//_thread = thread(&Loader::Loading(), this);

	return S_OK;
}

HRESULT Loader::Loading()
{
	HRESULT hr = CoInitializeEx(nullptr, 0);

	lock_guard<mutex> lock(_mutex);

	switch (nextLevelID)
	{
	case LEVEL::LOGO:
		Loading_Logo();
		break;

	case LEVEL::GAMEPLAY:
		Loading_GamePlay();
		break;
	}

	return S_OK;
}

void Loader::Loading_Logo()
{
	lstrcpy(loadingtxt, L"텍스처를 로딩중입니다");

	isFinished = true;
}

void Loader::Loading_GamePlay()
{

}
