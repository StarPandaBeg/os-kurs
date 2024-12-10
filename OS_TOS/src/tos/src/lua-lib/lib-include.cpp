#include "lua-lib/lib-include.h"

static const tos_global* m_os;

void arg_error(lua_State* L, const char* expected, int index);

void tos::lib::internal::setup(const tos_global& os)
{
	m_os = &os;
}

const tos_global& tos::lib::internal::os()
{
	return *m_os;
}

void tos::lib::internal::expect_arg(lua_State* L, int expected, bool strict)
{
	int n = lua_gettop(L);
	if (strict && n == expected) return;
	if (!strict && n >= expected) return;
	luaL_error(L, "Incorrect number of arguments. Expected %d, got %d.", expected, n);
}

double tos::lib::internal::get_number(lua_State* L, int index)
{
	if (!lua_isnumber(L, index)) arg_error(L, "number", index);
	return lua_tonumber(L, index);
}

bool tos::lib::internal::get_bool(lua_State* L, int index)
{
	if (!lua_isboolean(L, index)) arg_error(L, "boolean", index);
	return lua_toboolean(L, index);
}

const char* tos::lib::internal::get_string(lua_State* L, int index, bool strict)
{
	if (strict && lua_type(L, index) != LUA_TSTRING) arg_error(L, "string", index);
	if (!lua_isstring(L, index)) arg_error(L, "string", index);
	return lua_tostring(L, index);
}

void arg_error(lua_State* L, const char* expected, int index)
{
	luaL_error(L, "Argument %d must be a %s, got %s", index, expected, luaL_typename(L, index));
}
