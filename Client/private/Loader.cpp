#include "pch.h"
#include "Loader.h"

Loader::~Loader()
{
	StopAndJoin();
}

void Loader::Start(Job job)
{
	assert(!worker.joinable());

	stopRequested.store(false);
	{
		lock_guard<mutex> lock(mtx);
		done = false;
	}
	worker = thread([this, job] 
		{
			HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

			job(stopRequested);

			{
				lock_guard lock(mtx);
				done = true;
			}
			cv.notify_all();

			CoUninitialize();
		});
}

void Loader::WaitUntilDone()
{
	unique_lock<mutex> lock(mtx);
	cv.wait(lock, [this] { return done; });
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
