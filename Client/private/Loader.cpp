#include "pch.h"
#include "Loader.h"

Loader::~Loader()
{
	StopAndJoin();
}

void Loader::Start()
{
	stopRequested.store(false);
	{
		lock_guard<mutex> lock(mtx);
		done = false;
	}

	worker = thread([this] 
		{
			CoInitializeEx(nullptr, COINIT_MULTITHREADED);

			{
				lock_guard<mutex> lock(mtx);
				done = true;
			}
			cv.notify_one();

			CoUninitialize();
		});
}

void Loader::WaitUntilDone()
{
	unique_lock<mutex> lock(mtx);
	cv.wait(lock, [this] {return done; });
}

void Loader::RequestStop()
{
	stopRequested.store(true);
}

void Loader::StopAndJoin()
{
	RequestStop();
	if (worker.joinable())
		worker.join();
}

bool Loader::IsDone()
{
	lock_guard<mutex> lock(mtx);
	return done;
}
