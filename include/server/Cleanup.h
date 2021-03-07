#pragma once

#include <string>
#include <vector>

namespace spine {

	class Cleanup {
	public:
		static void init();

	private:
		static void cleanup();
		static void cleanupTable(const std::string & tableName, const std::vector<int> & userList);
	};

} /* namespace EW */
