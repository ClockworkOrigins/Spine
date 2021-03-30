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
// Copyright 2020 Clockwork Origins

#include "DatabaseMigrator.h"

#include <iostream>
#include <map>

#include "LanguageConverter.h"
#include "MariaDBWrapper.h"
#include "ServerCommon.h"
#include "SpineServerConfig.h"

using namespace spine;
using namespace spine::server;

void DatabaseMigrator::migrate() {
	do {
		CONNECTTODATABASE(__LINE__)

		if (!database.query("PREPARE selectProjectNamesStmt FROM \"SELECT * FROM projectNames\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectModNamesStmt FROM \"SELECT ModID, CAST(Name AS BINARY), Language FROM modnames\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE insertProjectNameStmt FROM \"INSERT INTO projectNames (ProjectID, Name, Languages) VALUES (?, CONVERT(? USING BINARY), ?)\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTeamNamesV2Stmt FROM \"SELECT * FROM teamNames\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTeamNamesStmt FROM \"SELECT TeamID, CAST(Name AS BINARY), Language FROM teamnames\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE insertTeamNameStmt FROM \"INSERT INTO teamNames (TeamID, Name, Languages) VALUES (?, CONVERT(? USING BINARY), ?)\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
		
		if (!database.query("EXECUTE selectProjectNamesStmt;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
		auto results = database.getResults<std::vector<std::string>>();
		
		if (results.empty()) {
			if (!database.query("EXECUTE selectModNamesStmt;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
				break;
			}
			results = database.getResults<std::vector<std::string>>();

			std::map<int, std::map<std::string, int>> nameMap;

			for (const auto & vec : results) {
				const int projectID = std::stoi(vec[0]);
				const std::string name = vec[1];
				const std::string languageString = vec[2];
				const int language = LanguageConverter::convert(languageString);

				nameMap[projectID][name] |= language;
			}

			for (auto & it : nameMap) {
				if (!database.query("SET @paramProjectID=" + std::to_string(it.first) + ";")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
					break;
				}
				for (auto & it2 : it.second) {
					if (!database.query("SET @paramName='" + it2.first + "';")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
						break;
					}
					if (!database.query("SET @paramLanguages=" + std::to_string(it2.second) + ";")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
						break;
					}
					if (!database.query("EXECUTE insertProjectNameStmt USING @paramProjectID, @paramName, @paramLanguages;")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
						break;
					}
				}
			}
		}
		
		if (!database.query("EXECUTE selectTeamNamesV2Stmt;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		
		if (results.empty()) {
			if (!database.query("EXECUTE selectTeamNamesStmt;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
				break;
			}
			results = database.getResults<std::vector<std::string>>();

			std::map<int, std::map<std::string, int>> nameMap;

			for (const auto & vec : results) {
				const int teamID = std::stoi(vec[0]);
				const std::string name = vec[1];
				const std::string languageString = vec[2];
				const int language = LanguageConverter::convert(languageString);

				nameMap[teamID][name] |= language;
			}

			for (auto & it : nameMap) {
				if (!database.query("SET @paramTeamID=" + std::to_string(it.first) + ";")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
					break;
				}
				for (auto & it2 : it.second) {
					if (!database.query("SET @paramName='" + it2.first + "';")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
						break;
					}
					if (!database.query("SET @paramLanguages=" + std::to_string(it2.second) + ";")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
						break;
					}
					if (!database.query("EXECUTE insertTeamNameStmt USING @paramTeamID, @paramName, @paramLanguages;")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
						break;
					}
				}
			}
		}
	} while (false);
	
	do {
		CONNECTTODATABASE(__LINE__)

		if (!database.query("PREPARE addColumnStmt FROM \"ALTER TABLE mods ADD SpineVersion INT NOT NULL DEFAULT 0\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
		
		if (database.query("SELECT SpineVersion FROM mods LIMIT 1;")) break;

		if (!database.query("ALTER TABLE mods ADD SpineVersion INT NOT NULL DEFAULT 0;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
	} while (false);
}
