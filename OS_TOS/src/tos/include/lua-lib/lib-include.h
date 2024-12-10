#pragma once

#pragma comment(lib,"lua54.lib")
extern "C"
{
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
}
#include "bootstrap.h"

#define luaL_insert_table_string(L, key, value) {lua_pushstring(L, key); lua_pushstring(L, value);lua_settable(L, -3);}
#define luaL_insert_table_number(L, key, value) {lua_pushstring(L, key); lua_pushinteger(L, value);lua_settable(L, -3);}

namespace tos {
	namespace lib {
		namespace internal {
			void setup(const tos_global& os);
			const tos_global& os();

			void expect_arg(lua_State* L, int n, bool strict = true);
			double get_number(lua_State* L, int index);
			bool get_bool(lua_State* L, int index);
			const char* get_string(lua_State* L, int index, bool strict = true);
		}
	}
}