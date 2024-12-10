#include "lua-lib/lib-io.h"

#include <conio.h>
#include <cctype>

using namespace tos;
using namespace tos::lib;
using namespace tos::lib::internal;

int tosopen_io_core(lua_State* L);

lua_CFunction tos::lib::tosopen_io(const tos_global& os)
{
	setup(os);
	return tosopen_io_core;
}

void check_proc_permission(std::string path, uint8_t what) {
    auto& executor = os().executor;
    auto& proc = executor->current();

    if (os().os->can(path, proc.realuser(), proc.groups(), what)) {
        return;
    }
    throw std::exception("no permission");
}

int to_offset_origin(std::string value) {
    if (value == "set") return FS_SEEK_SET;
    if (value == "cur") return FS_SEEK_CUR;
    if (value == "end") return FS_SEEK_END;
    throw std::exception("invalid origin value");
}

int io_open(lua_State* L) {
    expect_arg(L, 2);
    std::string path = get_string(L, 1);
    std::string mode_s = get_string(L, 2); // r, w, a, r+, w+, a+

    auto& executor = os().executor;

    try {
        auto& proc = executor->current();
        auto& fs = os().os->fs();
        auto mode = os().os->to_mode(mode_s);

        auto parent = fs::Path(path);
        parent.move_up();

        if (mode.write) {
            if (!os().os->can(parent.get(), proc.realuser(), proc.groups(), PERM_WRITE)) throw std::exception("no permission");
        }
       
        if (fs.exists(path)) {
            if (mode.read) check_proc_permission(path, PERM_READ);
            if (mode.write) check_proc_permission(path, PERM_WRITE);
        }

        auto handle = os().os->open(path, mode_s);
        proc.handle().insert(handle);

        fs.user(handle, proc.user());
        fs.group(handle, proc.group());

        lua_pushinteger(L, handle);
        return 1;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int io_close(lua_State* L) {
    expect_arg(L, 1);
    uint32_t handle = get_number(L, 1);

    auto& executor = os().executor;

    try {
        auto& proc = executor->current();
        if (proc.handle().count(handle) == 0) throw std::exception("invalid handle");

        os().os->close(handle);
        proc.handle().erase(handle);
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int io_read(lua_State* L) {
    expect_arg(L, 2);
    uint32_t handle = get_number(L, 1);
    uint32_t count = get_number(L, 2);

    auto& executor = os().executor;

    try {
        auto& proc = executor->current();
        if (proc.handle().count(handle) == 0) throw std::exception("invalid handle");

        std::vector<char> buffer(count);
        auto realcount = os().os->read(handle, buffer.data(), count);

        lua_pushlstring(L, buffer.data(), realcount);
        lua_pushinteger(L, realcount);
        return 2;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int io_write(lua_State* L) {
    expect_arg(L, 2);
    uint32_t handle = get_number(L, 1);
    const char* data = get_string(L, 2);
    uint32_t count = lua_rawlen(L, 2);

    auto& executor = os().executor;

    try {
        auto& proc = executor->current();
        if (proc.handle().count(handle) == 0) throw std::exception("invalid handle");
        os().os->write(handle, data, count);
        return 0;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int io_seek(lua_State* L) {
    expect_arg(L, 2, false);
    int n = lua_gettop(L);

    uint32_t handle = get_number(L, 1);
    uint32_t count = get_number(L, 2);
    std::string origin_s = (n == 3) ? get_string(L, 3) : "set";

    auto& executor = os().executor;

    try {
        auto& proc = executor->current();
        if (proc.handle().count(handle) == 0) throw std::exception("invalid handle");
        auto origin = to_offset_origin(origin_s);
        os().os->seek(handle, count, origin);
        return 0;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int io_tell(lua_State* L) {
    expect_arg(L, 1);
    uint32_t handle = get_number(L, 1);

    auto& executor = os().executor;

    try {
        auto& proc = executor->current();
        if (proc.handle().count(handle) == 0) throw std::exception("invalid handle");
        auto pos = os().os->tell(handle);
        lua_pushinteger(L, pos);
        return 1;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int io_end(lua_State* L) {
    expect_arg(L, 1);
    uint32_t handle = get_number(L, 1);

    auto& executor = os().executor;

    try {
        auto& proc = executor->current();
        if (proc.handle().count(handle) == 0) throw std::exception("invalid handle");
        auto pos = os().os->end(handle);
        lua_pushinteger(L, pos);
        return 1;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int io_getch(lua_State* L) {
    expect_arg(L, 0);

    try {
        int code = _getch();
        lua_pushinteger(L, code);
        if (code == 0 || code == 224) {
            lua_pushinteger(L, _getch());
        }
        else {
            lua_pushnil(L);
        }
        return 2;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int io_kbhit(lua_State* L) {
    expect_arg(L, 0);

    try {
        int value = _kbhit();
        lua_pushinteger(L, value);
        return 1;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int io_isprint(lua_State* L) {
    expect_arg(L, 1);
    unsigned char code = get_number(L, 1);

    try {
        lua_pushboolean(L, isprint(code));
        return 1;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int tosopen_io_core(lua_State* L)
{
    luaL_Reg mylib[] = {
        {"open", io_open},
        {"close", io_close},
        {"read", io_read},
        {"write", io_write},
        {"seek", io_seek},
        {"tell", io_tell},
        {"fend", io_end},
        {"getch", io_getch},
        {"kbhit", io_kbhit},
        {"isprint", io_isprint},
        {NULL, NULL}
    };
    luaL_newlib(L, mylib);
    return 1;
}
