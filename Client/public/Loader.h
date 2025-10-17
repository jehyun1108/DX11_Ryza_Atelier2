#pragma once

NS_BEGIN(Client)

class Loader
{
public:
	~Loader();

public:
	HRESULT Init(LEVEL nextLevelID);

	bool IsFinished() const { return isFinished; }
	void OutputLoadingText() { SetWindowText(g_hWnd, loadingtxt); }

private:
	HRESULT Loading();
	void Loading_Logo();
	void Loading_GamePlay();

private:
	GameInstance& game = GameInstance::GetInstance();
	bool isFinished = false;
	LEVEL nextLevelID = LEVEL::END;

	thread _thread;
	mutex _mutex;
	_tchar loadingtxt[MAX_PATH]{};
};

NS_END