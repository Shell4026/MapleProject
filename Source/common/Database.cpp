#include "Database.h"

#include "Core/Logger.h"

#include "Sqlite/sqlite3.h"

#include <limits>
namespace sh::game
{
	struct Database::Impl
	{
		sqlite3* db = nullptr;
	};

	Database::SQL::SQL(SQL&& other) noexcept
	{
		handle = other.handle;
		other.handle = nullptr;
	}

	Database::SQL::~SQL()
	{
		std::lock_guard<std::mutex> lock{ mu };
		if (handle != nullptr)
		{
			sqlite3_finalize(reinterpret_cast<sqlite3_stmt*>(handle));
			handle = nullptr;
		}
	}

	SH_USER_API auto Database::SQL::operator=(SQL&& other) noexcept -> SQL&
	{
		if (this == &other)
			return *this;

		if (handle != nullptr)
		{
			sqlite3_finalize(reinterpret_cast<sqlite3_stmt*>(handle));
			handle = nullptr;
		}

		handle = other.handle;
		other.handle = nullptr;
		return *this;
	}

	SH_USER_API auto Database::SQL::IsVaild() const -> bool
	{
		return handle != nullptr;
	}

	Database::Database(const std::filesystem::path& path)
	{
		impl = std::make_unique<Impl>();

		if (sqlite3_open(path.u8string().c_str(), &impl->db) != SQLITE_OK)
		{
			SH_ERROR_FORMAT("Failed to load DB: {}", path.u8string());
			if (impl->db != nullptr)
				sqlite3_close(impl->db);
			impl->db = nullptr;
		}
		else
			SH_INFO_FORMAT("Open DB: {}", path.u8string());

		SH_INFO_FORMAT("sqlite3_threadsafe() = {}", sqlite3_threadsafe());
	}
	Database::Database()
	{
		impl = std::make_unique<Impl>();
	}
	Database::~Database()
	{
		if (impl->db != nullptr)
		{
			sqlite3_close_v2(impl->db);
			impl->db = nullptr;
		}
	}
	SH_USER_API auto Database::Execute(const std::string& sql) const -> bool
	{
		if (impl->db == nullptr)
		{
			SH_ERROR("The database is not open!");
			return false;
		}
		char* errMsg = nullptr;
		if (sqlite3_exec(impl->db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) 
		{
			SH_ERROR_FORMAT("SQL Error:  {}", errMsg);
			sqlite3_free(errMsg);
			return false;
		}
		return true;
	}
	SH_USER_API auto Database::CreateSQL(const std::string& sql) const -> SQL
	{
		sqlite3_stmt* stmt = nullptr;
		if (sqlite3_prepare_v2(impl->db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
		{
			SH_ERROR_FORMAT("Failed to create SQL: {}", sql);
			SQL sql{};
			sql.handle = nullptr;
			return sql;
		}
		SQL resultSQL{};
		resultSQL.handle = stmt;
		return resultSQL;
	}

	SH_USER_API auto Database::IsOpen() const -> bool
	{
		return impl->db != nullptr;
	}

	void Database::BindParameter(const SQL& sql, int n, int64_t param) const
	{
		if (sql.handle == nullptr)
			return;
		sqlite3_bind_int64(reinterpret_cast<sqlite3_stmt*>(sql.handle), n, param);
	}
	void Database::BindParameter(const SQL& sql, int n, double param) const
	{
		if (sql.handle == nullptr)
			return;
		sqlite3_bind_double(reinterpret_cast<sqlite3_stmt*>(sql.handle), n, param);
	}
	void Database::BindParameter(const SQL& sql, int n, std::nullptr_t param) const
	{
		if (sql.handle == nullptr)
			return;
		sqlite3_bind_null(reinterpret_cast<sqlite3_stmt*>(sql.handle), n);
	}
	void Database::BindParameter(const SQL& sql, int n, std::string_view str) const
	{
		if (sql.handle == nullptr)
			return;
		int len = (str.size() > static_cast<size_t>(std::numeric_limits<int>::max())) ? 
			std::numeric_limits<int>::max() : static_cast<int>(str.size());

		sqlite3_bind_text(reinterpret_cast<sqlite3_stmt*>(sql.handle), n, str.data(), len, SQLITE_TRANSIENT); // 문자열 복사
	}
	auto Database::ExecuteSQL(const SQL& sql) const -> bool
	{
		int result = sqlite3_step(reinterpret_cast<sqlite3_stmt*>(sql.handle));

		sqlite3_reset(reinterpret_cast<sqlite3_stmt*>(sql.handle));
		sqlite3_clear_bindings(reinterpret_cast<sqlite3_stmt*>(sql.handle));

		if (result != SQLITE_DONE && result != SQLITE_ROW)
		{
			const char* errorMsg = sqlite3_errmsg(impl->db);
			SH_ERROR_FORMAT("SQLite Step Error: {}", errorMsg);
			return false;
		}
		return true;
	}
	auto Database::QuerySQL(const SQL& sql) const ->std::vector<std::vector<QueryResult>>
	{
		std::vector<std::vector<QueryResult>> fullResult;
		sqlite3_stmt* stmt = reinterpret_cast<sqlite3_stmt*>(sql.handle);

		const int columnCount = sqlite3_column_count(stmt);

		int rc = 0;
		while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
		{
			std::vector<QueryResult> rowData;
			rowData.reserve(columnCount);

			for (int i = 0; i < columnCount; ++i)
			{
				const int type = sqlite3_column_type(stmt, i);
				if (type == SQLITE_INTEGER)
					rowData.push_back(sqlite3_column_int64(stmt, i));
				else if (type == SQLITE_FLOAT)
					rowData.push_back(sqlite3_column_double(stmt, i));
				else if (type == SQLITE_TEXT)
				{
					const unsigned char* text = sqlite3_column_text(stmt, i);

					if (text)
						rowData.push_back(std::string(reinterpret_cast<const char*>(text)));
					else
						rowData.push_back(nullptr);
				}
				else
					rowData.push_back(nullptr);

			}
			fullResult.push_back(std::move(rowData));
		}
		if (rc != SQLITE_DONE)
			SH_ERROR_FORMAT("SQLite Query Error: {}", sqlite3_errmsg(impl->db));

		sqlite3_reset(stmt);
		sqlite3_clear_bindings(stmt);

		return fullResult;
	}
}//namespace