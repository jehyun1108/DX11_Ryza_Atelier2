#pragma once

#include "LoaderData.h"

NS_BEGIN(Client)

class Loader
{
public:
	using Job = function<void(atomic<bool>& stopRequested)>;

	~Loader();

public:
	void Start(Job job);   
	void WaitUntilDone();  
	void RequestStop();   
	void StopAndJoin();

	bool IsDone();  // non-blocking으로 끝났는지 확인

private:
	thread             worker;
	atomic<bool>       stopRequested = false;
	mutex              mtx;
	condition_variable cv;
	bool               done = false;
};

NS_END