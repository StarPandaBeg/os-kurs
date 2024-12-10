#pragma once

#include <cstdint>
#include "filesystem.h"

namespace tos {
	typedef struct user_entry {
		uint16_t u_uid;
		char u_name[30];
		char u_password[32];
		char u_home[64];
	} user_entry;

	class UserTable {
	public:
		UserTable(fs::Filesystem& fs, std::string filepath = "/etc/passwd");
	};
}