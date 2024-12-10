#pragma once

#include <cstdint>
#include <map>
#include <set>
#include <string>

#pragma comment(lib,"lua54.lib")
extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace tos {
	class Process {
	public:
		Process();
		Process(std::string path);

		uint16_t user() const;
		void user(uint16_t value);

		uint16_t group() const;
		void group(uint16_t value);

		uint16_t realuser() const;
		void realuser(uint16_t value);

		const std::string& path() const;

		lua_State* state() const;
		void state(lua_State* state);

		std::set<uint32_t>& handle();
		std::set<uint16_t>& groups();

		bool hasenv(std::string key);
		std::map<std::string, std::string>& env();
		std::string env(std::string key);
		void env(std::string key, std::string value);
	private:
		uint16_t m_uid;
		uint16_t m_gid;
		uint16_t m_euid;
		std::string m_path;
		std::map<std::string, std::string> m_env;
		std::set<uint32_t> m_handle;
		std::set<uint16_t> m_groups;
		lua_State* m_state = nullptr;
	};
}