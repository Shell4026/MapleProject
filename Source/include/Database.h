#pragma once
#include "Export.h"

#include <filesystem>
#include <memory>
#include <string>
#include <type_traits>
#include <cstdint>
#include <string_view>
#include <mutex>
#include <variant>
namespace sh::game
{
	class Database
	{
	public:
		class SQL
		{
			friend Database;
		public:
			SH_USER_API SQL() = default;
			SH_USER_API SQL(const SQL& other) = delete;
			SH_USER_API SQL(SQL&& other) noexcept;
			SH_USER_API ~SQL();

			SH_USER_API auto operator=(const SQL& other) -> SQL& = delete;
			SH_USER_API auto operator=(SQL&& other) noexcept -> SQL&;

			SH_USER_API auto IsVaild() const -> bool;
		private:
			void* handle = nullptr;
			mutable std::mutex mu;
		};
		using QueryResult = std::variant<int64_t, double, std::string, std::nullptr_t>;
	public:
		SH_USER_API Database(const std::filesystem::path& path);
		SH_USER_API Database();
		SH_USER_API ~Database();

		SH_USER_API auto Execute(const std::string& sql) const -> bool;

		SH_USER_API auto CreateSQL(const std::string& sql) const -> SQL;

		SH_USER_API auto IsOpen() const -> bool;

		template<typename... Args>
		SH_USER_API auto Execute(const SQL& sql, Args&&... args) const -> bool;
		template<typename... Args>
		SH_USER_API auto Query(const SQL& sql, Args&&... args) const -> std::vector<std::vector<QueryResult>>;
	private:

		void BindParameter(const SQL& sql, int n, int64_t param) const;
		void BindParameter(const SQL& sql, int n, double param) const;
		void BindParameter(const SQL& sql, int n, std::nullptr_t param) const;
		void BindParameter(const SQL& sql, int n, std::string_view str) const;

		auto ExecuteSQL(const SQL& sql) const -> bool;
		auto QuerySQL(const SQL& sql) const -> std::vector<std::vector<QueryResult>>;
	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};

	template<typename... Args>
	inline auto Database::Execute(const SQL& sql, Args&&... args) const -> bool
	{
		std::lock_guard<std::mutex> lock{ sql.mu };
		int idx = 1;

		const auto loopFn = 
			[&](auto&& a)
			{
				using T = std::decay_t<decltype(a)>;

				if constexpr (std::is_integral_v<T>)
					BindParameter(sql, idx++, static_cast<int64_t>(a));
				else if constexpr (std::is_floating_point_v<T>)
					BindParameter(sql, idx++, static_cast<double>(a));
				else
					BindParameter(sql, idx++, std::forward<decltype(a)>(a));
			};

		(loopFn(std::forward<Args>(args)), ...);

		return ExecuteSQL(sql);
	}
	template<typename... Args>
	inline auto Database::Query(const SQL& sql, Args&& ...args) const -> std::vector<std::vector<Database::QueryResult>>
	{
		std::lock_guard<std::mutex> lock{ sql.mu };
		int idx = 1;

		const auto loopFn =
			[&](auto&& a)
			{
				using T = std::decay_t<decltype(a)>;

				if constexpr (std::is_integral_v<T>)
					BindParameter(sql, idx++, static_cast<int64_t>(a));
				else if constexpr (std::is_floating_point_v<T>)
					BindParameter(sql, idx++, static_cast<double>(a));
				else
					BindParameter(sql, idx++, std::forward<decltype(a)>(a));
			};

		(loopFn(std::forward<Args>(args)), ...);

		return QuerySQL(sql);
	}
}//namespace