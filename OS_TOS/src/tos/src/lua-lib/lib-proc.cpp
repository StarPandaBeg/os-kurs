#include "lua-lib/lib-proc.h"

using namespace tos;
using namespace tos::lib;
using namespace tos::lib::internal;

int tosopen_proc_core(lua_State* L);

lua_CFunction tos::lib::tosopen_proc(const tos_global& os) {
    setup(os);
    return tosopen_proc_core;
}

int proc_spawn(lua_State* L) {
    expect_arg(L, 1);
    std::string procPath = get_string(L, 1);

    try {
        auto& proc = os().executor->current();
        if (!os().os->can(procPath, proc.realuser(), proc.groups(), PERM_EXECUTE)) 
            throw std::exception("no permission");

        auto pid = os().executor->spawn(procPath);
        lua_pushinteger(L, pid);
        return 1;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int proc_suspawn(lua_State* L) {
    expect_arg(L, 3);
    std::string procPath = get_string(L, 1);
    uint16_t uid = get_number(L, 2);
    uint16_t euid = get_number(L, 3);

    try {
        auto& proc = os().executor->current();
        if (!os().os->can(procPath, proc.realuser(), proc.groups(), PERM_EXECUTE)) 
            throw std::exception("no permission");

        auto pid = os().executor->suspawn(procPath, uid, euid);
        lua_pushinteger(L, pid);
        return 1;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int proc_execute(lua_State* L) {
    expect_arg(L, 1);
    int pid = get_number(L, 1);

    try {
        os().executor->execute(pid);
        lua_pushboolean(L, true);
        lua_pushnil(L);
    }
    catch (std::exception& e) {
        lua_pushboolean(L, false);
        lua_pushstring(L, e.what());
    }
    return 2;
}

int proc_env(lua_State* L) {
    int n = lua_gettop(L);

    expect_arg(L, 1, false);
    int pid = get_number(L, 1);

    try {
        auto& proc = os().executor->get(pid);

        // Get value
        if (n == 2) {
            std::string key = get_string(L, 2);
            if (!proc.hasenv(key)) {
                lua_pushnil(L);
                return 1;
            }
            lua_pushstring(L, proc.env(key).c_str());
            return 1;
        }
        // Set value
        if (n == 3) {
            std::string key = get_string(L, 2);
            std::string value = get_string(L, 3);
            proc.env(key, value);
            return 0;
        }
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int proc_group(lua_State* L) {
    expect_arg(L, 2);
    int pid = get_number(L, 1);
    int gid = get_number(L, 2);

    try {
        auto& proc = os().executor->get(pid);
        proc.group(gid);
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int proc_groups(lua_State* L) {
    expect_arg(L, 2);
    int pid = get_number(L, 1);

    if (!lua_istable(L, 2)) {
        return luaL_error(L, "Invalid argument 2: groups must be a table");
    }

    std::set<uint16_t> groups;
    lua_pushnil(L);
    while (lua_next(L, 2) != 0) {
        if (lua_isnumber(L, -1)) {
            groups.insert(lua_tointeger(L, -1));
        }
        else {
            luaL_error(L, "Invalid argument 2: groups must be a table of numbers");
        }
        lua_pop(L, 1);
    }

    try {
        auto& proc = os().executor->get(pid);
        proc.groups().clear();
        for (auto g : groups) {
            proc.groups().insert(g);
        }
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int proc_pid(lua_State* L) {
    expect_arg(L, 0);
    try {
        auto pid = os().executor->current_pid();
        lua_pushinteger(L, pid);
        return 1;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int proc_info(lua_State* L) {
    expect_arg(L, 0);
    try {
        auto proc = os().executor->current();
        lua_newtable(L);
        luaL_insert_table_number(L, "uid", proc.user());
        luaL_insert_table_number(L, "gid", proc.group());
        luaL_insert_table_number(L, "euid", proc.realuser());
        return 1;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int tosopen_proc_core(lua_State* L)
{
    luaL_Reg mylib[] = {
            {"spawn", proc_spawn},
            {"suspawn", proc_suspawn},
            {"execute", proc_execute},
            {"env", proc_env},
            {"pid", proc_pid},
            {"info", proc_info},
            {"group", proc_group},
            {"groups", proc_groups},
            {NULL, NULL}
    };
    luaL_newlib(L, mylib);
	return 1;
}
