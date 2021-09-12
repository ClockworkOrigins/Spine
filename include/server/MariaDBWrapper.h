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

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "mariadb/mysql.h"

namespace spine {
namespace server {

	class MariaDBWrapper {
	public:
		MariaDBWrapper();

		~MariaDBWrapper() {
			close();
		}

		bool connect(const std::string & host, const std::string & user, const std::string & password, const std::string & database, uint16_t port);

		bool query(const std::string & query) const;

		template<typename Result>
		std::vector<Result> getResults() {
			if (!_database) {
				return{};
			}
			MYSQL_RES * res = mysql_store_result(_database);

			std::vector<Result> result;

			MYSQL_ROW row;

			const unsigned int fields = mysql_num_fields(res);

			while ((row = mysql_fetch_row(res)) != nullptr) {
				std::vector<std::string> params;

				for (unsigned int i = 0; i < fields; i++) {
					params.push_back(row[i]);
				}

				result.push_back(Result(params));
			}

			mysql_free_result(res);

			return result;
		}

		void close();

	private:
		MYSQL * _database;
	};

} /* namespace server */
} /* namespace spine */
