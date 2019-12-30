/*
	This file is part of Spine.

    Spine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Spine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Spine.  If not, see <http://www.gnu.org/licenses/>.
 */
// Copyright 2018 Clockwork Origins

#ifndef __SPINE_DATABASE_H__
#define __SPINE_DATABASE_H__

#include <map>
#include <mutex>
#include <tuple>
#include <vector>

#include <QtGlobal>

#include "sqlite3.h"

class Database {
public:
	/**
	 * \brief struct to store errors
	 * error flag indicates whether an error occured.
	 * if the error occured, a more sophisticated message is available in errorMsg
	 * if no error occured, the errorMsg string will not be modified and thus can contain arbitary content
	 */
	struct DBError {
		DBError() : error(false) {}
		bool error;
		std::string errMsg;
	};

	/**
	 * \brief executes an arbitary sql operation
	 * \param[in] dbpath filename of the database to be used
	 * \param[in] query Query string to be executed
	 */
	static void execute(const std::string & dbpath, const std::string & query, DBError & error);

	/**
	 * \brief opens given database and keeps it open
	 * \param[in] dbpath filename of the database to be used
	 */
	static void open(const std::string & dbpath, DBError & error);

	/**
	 * \brief closes given database
	 * \param[in] dbpath filename of the database to be used
	 */
	static void close(const std::string & dbpath, DBError & error);

	/**
	 * \brief returns the nth element of a query (0-indexed)
	 * \param[in] dbpath path to the database file
	 * \param[in] query query to execute
	 * \param[out] error strcut containing the result code
	 * \param[in] n index of row to return
	 * \returns requested row
	 * if an error occured, an default constructed Result object is returned
	 */
	template<typename Result, typename... Columns>
	static Result queryNth(std::string dbpath, std::string query, DBError & error, unsigned int n = 0) {
		sqlite3 * db;
		sqlite3_stmt * stmt;
		std::lock_guard<std::mutex> lg(_lock); // _lock the database vector and sqlite
		int r = sqlite3_open_v2(dbpath.c_str(), &db, SQLITE_OPEN_READONLY, nullptr);
		if (r != SQLITE_OK) {
			error.error = true;
			error.errMsg = std::string("open(): ") + sqlite3_errmsg(db);
			return Result();
		}
		r = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
		if (r != SQLITE_OK) {
			error.error = true;
			error.errMsg = std::string("prepare(): ") + sqlite3_errmsg(db);
			return Result();
		}
		for (unsigned int i = 0; i <= n; i++) {
			r = sqlite3_step(stmt);
			if (r != SQLITE_ROW) {
				error.error = true;
				if (r == SQLITE_DONE) {
					error.errMsg = std::string("not enough rows available");
				} else {
					error.errMsg = std::string("step(): ") + sqlite3_errmsg(db);
				}
				return Result();
			}
		}

		ColumnRetriever<sizeof...(Columns) - 1, Columns...> colRet;
		std::tuple<Columns...> resultColumns = colRet.retrieve(stmt);

		Result ret = tupleToResult<Result>(resultColumns, typename GenerateSequence<sizeof...(Columns)>::type());
		if (sqlite3_errcode(db) != SQLITE_OK && sqlite3_errcode(db) != SQLITE_ROW && sqlite3_errcode(db) != SQLITE_DONE) {
			error.error = true;
			error.errMsg = std::string("column_*(): ") + sqlite3_errstr(sqlite3_errcode(db)); // sqlite3_errmsg() doesn't work here, because a later call to queryColumn may have succeeded
			return Result();
		}
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		error.error = false;
		return ret;
	}

	/**
	 * \brief returns all queried rows
	 * \param[in] dbpath path to the database file
	 * \param[in] query query to execute
	 * \param[out] error strcut containing the result code
	 * \returns vector containing all rows
	 * if an error occured, the returned vector may still contain some data, but
	 * it is not guaranteed that the data is complete or correct
	 */
	template<typename Result, typename... Columns>
	static std::vector<Result> queryAll(std::string dbpath, std::string query, DBError & error) {
		sqlite3 * db;
		sqlite3_stmt * stmt;
		std::vector<Result> v;
		bool selfOpened = false;
		std::lock_guard<std::mutex> lg(_lock); // _lock the database vector and sqlite
		if (_databases.find(dbpath) != _databases.end()) {
			db = _databases[dbpath];
		} else {
			int r = sqlite3_open_v2(dbpath.c_str(), &db, SQLITE_OPEN_READONLY, nullptr);
			if (r != SQLITE_OK) {
				error.error = true;
				error.errMsg = std::string("open(): ") + sqlite3_errmsg(db);
				return v;
			}
			selfOpened = true;
		}
		int r = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
		if (r != SQLITE_OK) {
			error.error = true;
			error.errMsg = std::string("prepare(): ") + sqlite3_errmsg(db);
			return v;
		}
		while ((r = sqlite3_step(stmt)) == SQLITE_ROW) {
			DBError err;

			ColumnRetriever<sizeof...(Columns) - 1, Columns...> colRet;
			std::tuple<Columns...> resultColumns = colRet.retrieve(stmt);

			Result ret = tupleToResult<Result>(resultColumns, typename GenerateSequence<sizeof...(Columns)>::type());
			if (sqlite3_errcode(db) != SQLITE_OK && sqlite3_errcode(db) != SQLITE_ROW && sqlite3_errcode(db) != SQLITE_DONE) {
				error.error = true;
				error.errMsg = std::string("column_*(): ") + sqlite3_errstr(sqlite3_errcode(db)); // sqlite3_errmsg() doesn't work here, because a later call to queryColumn may have succeeded
				return v;
			}
			v.push_back(ret);
		}
		if (r != SQLITE_DONE) {
			error.error = true;
			error.errMsg = std::string("step(): ") + sqlite3_errmsg(db);
			return v;
		}
		sqlite3_finalize(stmt);

		if (selfOpened) {
			sqlite3_close(db);
		}
		error.error = false;
		return v;
	}

	/**
	 * \brief returns the number of result rows
	 * \param[in] dbpath path to the database file
	 * \param[in] query query to execute
	 * \param[out] error strcut containing the result code
	 * \returns number of rows the query produces, or -1 if an error occured
	 */
	static int queryCount(std::string dbpath, std::string query, DBError & error) {
		sqlite3 * db;
		sqlite3_stmt * stmt;
		std::lock_guard<std::mutex> lg(_lock); // _lock the database vector and sqlite
		int r = sqlite3_open_v2(dbpath.c_str(), &db, SQLITE_OPEN_READONLY, nullptr);
		if (r != SQLITE_OK) {
			error.error = true;
			error.errMsg = std::string("open(): ") + sqlite3_errmsg(db);
			return -1;
		}
		r = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
		if (r != SQLITE_OK) {
			error.error = true;
			error.errMsg = std::string("prepare(): ") + sqlite3_errmsg(db);
			return -1;
		}
		int counter = 0;
		while((r = sqlite3_step(stmt)) == SQLITE_ROW) {
			counter++;
		}
		if (r != SQLITE_DONE) {
			error.error = true;
			error.errMsg = std::string("step(): ") + sqlite3_errmsg(db);
			return -1;
		}
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		error.error = false;
		return counter;
	}

	/**
	 * \brief returns whether the given Database exists and can be opend
	 */
	static bool exists(std::string dbpath) {
		sqlite3 * db;
		std::lock_guard<std::mutex> lg(_lock); // _lock the database vector and sqlite
		int r = sqlite3_open_v2(dbpath.c_str(), &db, SQLITE_OPEN_READONLY, nullptr);
		if (r != SQLITE_OK) {
			return false;
		}
		sqlite3_close(db);
		return true;
	}

private:
	 // _lock the database vector and sqlite
	static std::mutex _lock;

	template<typename T>
	static T queryColumn(int column, sqlite3_stmt * stmt) {
		return T();
	}

	/**
	 * \brief default case for recursion.
	 * This struct will retrieve one Column, index with Index
	 * \param[in] Template:Index index of the Column to be used
	 * \param[in] Template:LastColumn type of this last column
	 * \param[in] Template:NullColumn only matches an empty list
	 */
	template<int Index, typename LastColumn, typename... NullColumn>
	struct ColumnRetriever {
		std::tuple<LastColumn> retrieve(sqlite3_stmt * stmt) {
			return std::tuple<LastColumn>(queryColumn<LastColumn>(Index, stmt));
		}
	};

	/**
	 * \brief recursive struct. retrieves all columns
	 * This struct will retrieve all the columns
	 * Itself only Column1 whereas the recursion will retrieve the remaining Column2 and OtherColumns
	 * Note: Column2 is needed for a correct recursion. There's no other reason why to seperate it from OtherColumns
	 * \param[in] Template:Index index of the current column to be used
	 * \param[in] Template:Column1 type of this column
	 * \param[in] Template:Column2 type of the next column
	 * \param[in] Template:OtherColumns types of the other remaining columns
	 */
	template<int Index, typename Column1, typename Column2, typename... OtherColumns>
	struct ColumnRetriever<Index, Column1, Column2, OtherColumns...> {
		std::tuple<Column1, Column2, OtherColumns...> retrieve(sqlite3_stmt * stmt) {
			ColumnRetriever<Index - 1, Column2, OtherColumns...> colRet;
			return std::tuple_cat(colRet.retrieve(stmt), std::tuple<Column1>(queryColumn<Column1>(Index, stmt)));
		}
	};

	/**
	 * \brief represents an integer_sequence
	 * here used as <0, 1, 2, ...>
	 */
	template<int...>
	struct intSeq {
	};

	/**
	 * \brief generates an integer_sequence
	 * It will generate a sequence from 0 to N-1 and prepend it to S
	 * \param[in] Template:N amount of remaining integers
	 * \param[in] Template:S liste of already generated integers
	 */
	template<int N, int... S>
	struct GenerateSequence : GenerateSequence<N-1, N-1, S...> {
	};

	/**
	 * \brief generates an integer_sequence; default case
	 * It will prepend a '0' to the list and define the type
	 * \param[in] Template:S liste of already generated integers
	 */
	template<int... S>
	struct GenerateSequence<0, S...> {
		typedef intSeq<S...> type; // type: intSeq<0, 1, 2, ...>
	};

	/**
	 * \brief unpacks a tuple and constructs a Result object
	 * Note: only the first Template argument has to be explicitly specified during a call. The others should be auto-detectable
	 * \param[in] Template:Result type of return value, make sure it has a constructor taking the correct values
	 * \param[in] Template:Tuple the tupletype to be unpacked
	 * \param[in] Template:S integer_sequence to unpack the tuple
	 */
	template<typename Result, typename Tuple, int... S>
	static typename std::enable_if<std::is_fundamental<Result>::value, Result>::type tupleToResult(Tuple & tpl, intSeq<S...> s) {
		static_cast<void>(s);
		return Result(std::get<S>(tpl)...);
	}

	template<typename Result, typename Tuple, int... S>
	static typename std::enable_if<!std::is_fundamental<Result>::value, Result>::type tupleToResult(Tuple & tpl, intSeq<S...> s) {
		static_cast<void>(s);
		return Result({std::get<S>(tpl)...});
	}

	static std::map<std::string, sqlite3 *> _databases;
};

template<>
int Database::queryColumn<int>(int column, sqlite3_stmt * stmt);

template<>
std::string Database::queryColumn<std::string>(int column, sqlite3_stmt * stmt);

#endif /* __SPINE_DATABASE_H__ */
