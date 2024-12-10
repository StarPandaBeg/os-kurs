#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#pragma comment(lib,"lua54.lib")
extern "C"
{
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
}

namespace tos {
	class LuaRegistry {
	public:
		lua_State* create_state_for(uint16_t pid);

		void register_lib(std::string name, lua_CFunction library);
		void unregister_lib(std::string name);

		void register_searcher(lua_CFunction searcher);

		LuaRegistry(LuaRegistry const&) = delete;
		void operator=(LuaRegistry const&) = delete;
		static LuaRegistry& instance();
	private:
		LuaRegistry() {}
		std::map<std::string, lua_CFunction> m_libraries;
		std::vector<lua_CFunction> m_searchers;

		void state_setup_libs(lua_State* L);
		void state_setup_searchers(lua_State* L);
	};
}