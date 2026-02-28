#include "System/DBThread.h"

namespace sh::game
{
	DBThread::~DBThread()
	{
		Stop();
		cvTask.notify_all();
		if (thr.joinable())
			thr.join();
	}
	SH_USER_API auto DBThread::Init(const std::filesystem::path& dbPath) -> bool
	{
		if (bInit)
			return true;

		if (!db.Open(dbPath))
			return false;

		thr = std::thread(
			[this]()
			{
				while (true)
				{
					std::function<void()> task;
					{
						std::unique_lock<std::mutex> lock{ mu };

						cvTask.wait(lock, [this]() {return bStop || !tasks.empty(); });

						if (bStop && tasks.empty())
							return;

						task = std::move(tasks.front());
						tasks.pop();
					}

					task();
				}
			}
		);
		bInit = true;
		return true;
	}
	SH_USER_API void DBThread::Stop()
	{
		{
			std::lock_guard<std::mutex> lock{ mu };
			bStop = true;
		}
		cvTask.notify_all();
	}
	SH_USER_API auto DBThread::CreateSQL(std::string_view sql) -> Database::SQL
	{
		if (!db.IsOpen())
			return {};
		return db.CreateSQL(sql);
	}
	DBThread::DBThread()
	{
	}
}//namespace