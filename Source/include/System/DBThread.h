#pragma once
#include "Export.h"
#include "Database.h"

#include "core/Singleton.hpp"

#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <future>
#include <type_traits>
#include <filesystem>
#include <string_view>
namespace sh::game
{
	class DBThread : public core::Singleton<DBThread>
	{
		friend Singleton<DBThread>;
	public:
		SH_USER_API ~DBThread();

		SH_USER_API auto Init(const std::filesystem::path& dbPath) -> bool;
		SH_USER_API void Stop();
		SH_USER_API auto CreateSQL(std::string_view sql) -> Database::SQL;

		/// @brief 해당 db는 DBThread에서만 사용할 것
		SH_USER_API auto GetDB() -> Database& { return db; }

		template<typename F, typename ...Args>
		auto AddTask(F&& func, Args&& ...args) -> std::future<typename std::invoke_result_t<F, Args...>>;
	protected:
		SH_USER_API DBThread();
	private:
		std::mutex mu;
		std::thread thr;
		std::condition_variable cvTask;
		std::queue<std::function<void()>> tasks;

		Database db;

		bool bInit = false;
		bool bStop = false;
	};

	template<typename F, typename ...Args>
	inline auto DBThread::AddTask(F&& func, Args&& ...args) -> std::future<typename std::invoke_result_t<F, Args...>>
	{
		using ReturnType = typename std::invoke_result_t<F, Args...>;
		using PackageType = std::packaged_task<ReturnType()>;

		auto packagePtr = new PackageType(std::bind(std::forward<F>(func), std::forward<Args>(args)...));
		auto result = packagePtr->get_future();
		{
			std::lock_guard<std::mutex> lock{ mu };
			auto task =
				[packagePtr]()
				{
					std::unique_ptr<PackageType> uniquePackage{ packagePtr };
					(*packagePtr)();
				};
			tasks.push(std::move(task));
		}

		cvTask.notify_one();

		return result;
	}
}//namespace