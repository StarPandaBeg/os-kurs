#include "lua-registry.h"

using namespace tos;

struct luaL_Reg_ext {
	const char* name;
	lua_CFunction func;
	int glb;
};

lua_State* LuaRegistry::create_state_for(uint16_t pid)
{
	auto state = luaL_newstate();
	state_setup_libs(state);
	state_setup_searchers(state);
	return state;
}

void LuaRegistry::register_lib(std::string name, lua_CFunction library)
{
	m_libraries.emplace(std::make_pair(name, library));
}

void LuaRegistry::unregister_lib(std::string name)
{
	m_libraries.erase(name);
}

void LuaRegistry::register_searcher(lua_CFunction searcher)
{
	m_searchers.push_back(searcher);
}

LuaRegistry& LuaRegistry::instance()
{
	static LuaRegistry instance;
	return instance;
}

void LuaRegistry::state_setup_libs(lua_State* L)
{
	static const luaL_Reg_ext lualibs[] =
	{
		{"base", luaopen_base, 1},
		{"package", luaopen_package, 1},
		{"io-real", luaopen_io, 0},
		{"math", luaopen_math, 0},
		{"os-real", luaopen_os, 0},
		{"string", luaopen_string, 0},
		{"table", luaopen_table, 0},
		{NULL, NULL, 0}
	};

	for (const luaL_Reg_ext* lib = lualibs; lib->func != NULL; lib++)
	{
		luaL_requiref(L, lib->name, lib->func, lib->glb);
		lua_settop(L, 0);
	}

	/*lua_register(L, "error", [](lua_State* L) {
		
		return 0;
	});*/

	for (auto const& pair : m_libraries) {
		luaL_requiref(L, pair.first.c_str(), pair.second, 0);
		lua_settop(L, 0);
	}
}

void LuaRegistry::state_setup_searchers(lua_State* L)
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "searchers");

	for (int i = 0; i < m_searchers.size(); ++i) {
		lua_pushcfunction(L, m_searchers[i]);
		lua_rawseti(L, -2, 2 + i);
	}

	lua_settop(L, 0);
}
