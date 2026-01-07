#pragma once
#include "Export.h"

#include <filesystem>
#include <memory>
#include <string>
#include <type_traits>
#include <cstdint>
#include <string_view>
namespace sh::game
{
	class Database
	{
	public:
		struct SQL
		{
			void* handle = nullptr;

			SH_USER_API ~SQL();
		};
	public:
		SH_USER_API Database(const std::filesystem::path& path);
		SH_USER_API Database();
		SH_USER_API ~Database();

		SH_USER_API auto Execute(const std::string& sql) const -> bool;

		SH_USER_API auto CreateSQL(const std::string& sql) const -> SQL;

		template<typename... Args>
		SH_USER_API auto Execute(const SQL& sql, Args&&... args) const -> bool;
		template<typename... Args>
		SH_USER_API auto Query(const SQL& sql, Args&&... args) const -> std::vector<std::vector<std::string>>;
	private:
		void BindParameter(const SQL& sql, int n, int param) const;
		void BindParameter(const SQL& sql, int n, int64_t param) const;
		void BindParameter(const SQL& sql, int n, double param) const;
		void BindParameter(const SQL& sql, int n, std::nullptr_t param) const;
		void BindParameter(const SQL& sql, int n, std::string_view str) const;
		auto ExecuteSQL(const SQL& sql) const -> bool;
		auto QuerySQL(const SQL& sql) const -> std::vector<std::vector<std::string>>;
	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};

	template<typename... Args>
	inline auto Database::Execute(const SQL& sql, Args&&... args) const -> bool
	{
		int idx = 1;
		(BindParameter(sql, idx++, std::forward<Args>(args)), ...);

		return ExecuteSQL(sql);
	}
	template<typename... Args>
	inline auto Database::Query(const SQL& sql, Args&& ...args) const ->std::vector<std::vector<std::string>>
	{
		int idx = 1;
		(BindParameter(sql, idx++, std::forward<Args>(args)), ...);

		return QuerySQL(sql);
	}
}//namespace