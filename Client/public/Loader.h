#pragma once

#include "LoaderData.h"

NS_BEGIN(Client)

class Loader
{
public:
	~Loader();

public:
	void Start();
	void WaitUntilDone();
	void RequestStop();
	void StopAndJoin();

	bool IsDone();

private:
	thread             worker;
	atomic<bool>       stopRequested = false;
	mutex              mtx;
	condition_variable cv;

	bool               done = false;
};

NS_END