#include "lua-lib/lib-os.h"
#include "attr.h"

#include <chrono>
#include <thread>

using namespace tos;
using namespace tos::lib;
using namespace tos::lib::internal;

int tosopen_os_core(lua_State* L);

lua_CFunction tos::lib::tosopen_os(const tos_global& os)
{
    setup(os);
    return tosopen_os_core;
}

int os_exists(lua_State* L) {
    expect_arg(L, 1);
    std::string path = get_string(L, 1);

    try {
        auto& fs = os().os->fs();
        auto exists = fs.exists(path);
        lua_pushboolean(L, exists);
        return 1;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int os_isfile(lua_State* L) {
    expect_arg(L, 1);
    std::string path = get_string(L, 1);

    try {
        auto& fs = os().os->fs();
        auto exists = fs.is_file(path);
        lua_pushboolean(L, exists);
        return 1;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int os_iscatalog(lua_State* L) {
    expect_arg(L, 1);
    std::string path = get_string(L, 1);

    try {
        auto& fs = os().os->fs();
        auto exists = fs.is_catalog(path);
        lua_pushboolean(L, exists);
        return 1;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int os_listdir(lua_State* L) {
    expect_arg(L, 1);
    std::string path = get_string(L, 1);

    try {
        auto& proc = os().executor->current();
        auto& fs = os().os->fs();
        auto exists = fs.is_catalog(path);
        if (!exists) throw std::exception("not a directory");
        if (!os().os->can(path, proc.realuser(), proc.groups(), PERM_READ)) throw std::exception("no permission");

        auto entries = fs.list_catalog(path);

        lua_newtable(L);
        int i = 1;
        for (auto entry : entries) {
            lua_pushnumber(L, i++);
            lua_pushstring(L, entry.c_str());
            lua_settable(L, -3);
        }
        return 1;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int os_mkdir(lua_State* L) {
    expect_arg(L, 1);
    std::string path = get_string(L, 1);

    try {
        auto& proc = os().executor->current();
        auto& fs = os().os->fs();
        auto exists = fs.exists(path);
        if (exists) throw std::exception("already exists");

        auto parent = fs::Path(path);
        parent.move_up();

        if (!os().os->can(parent.get(), proc.realuser(), proc.groups(), PERM_WRITE)) throw std::exception("no permission");
        fs.create_catalog(path, proc.user(), proc.group());
        return 0;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int os_clear(lua_State* L) {
    expect_arg(L, 0);

    try {
        system("cls||clear");
        return 0;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int os_filedata(lua_State* L) {
    expect_arg(L, 1);
    std::string path = get_string(L, 1);

    try {
        auto& proc = os().executor->current();
        auto& fs = os().os->fs();
        auto exists = fs.exists(path);
        if (!exists) throw std::exception("not found");

        auto parent = fs::Path(path);
        parent.move_up();
        if (!os().os->can(parent.get(), proc.realuser(), proc.groups(), PERM_READ)) throw std::exception("no permission");

        auto filedata = fs.filedata(path);

        lua_newtable(L);
        luaL_insert_table_number(L, "attr", filedata.fs_attr);
        luaL_insert_table_number(L, "links", filedata.fs_links);
        luaL_insert_table_number(L, "uid", filedata.fs_uid);
        luaL_insert_table_number(L, "gid", filedata.fs_gid);
        luaL_insert_table_number(L, "size", filedata.fs_size);
        luaL_insert_table_number(L, "fsize", filedata.fs_fsize);
        luaL_insert_table_number(L, "ctime", filedata.fs_ctime);
        luaL_insert_table_number(L, "mtime", filedata.fs_mtime);

        return 1;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int os_chmod(lua_State* L) {
    expect_arg(L, 2);
    std::string path = get_string(L, 1);
    uint16_t mode = get_number(L, 2);

    try {
        auto& proc = os().executor->current(); 
        auto& fs = os().os->fs();
        auto exists = fs.exists(path);
        if (!exists) throw std::exception("not found");
        auto data = fs.filedata(path);
        auto attr = fs::FileAttr(data.fs_attr);

        if (data.fs_uid != proc.realuser() && proc.realuser() != 0) throw std::exception("no permission");

        uint8_t owner = (mode / 100) % 10;     // Первая цифра
        uint8_t group = (mode / 10) % 10;      // Вторая цифра
        uint8_t world = mode % 10;            // Третья цифра

        attr.can(0, 0, owner & 1 << 2);
        attr.can(0, 1, owner & 1 << 1);
        attr.can(0, 2, owner & 1 << 0);
        attr.can(1, 0, group & 1 << 2);
        attr.can(1, 1, group & 1 << 1);
        attr.can(1, 2, group & 1 << 0);
        attr.can(2, 0, world & 1 << 2);
        attr.can(2, 1, world & 1 << 1);
        attr.can(2, 2, world & 1 << 0);

        auto handle = fs.open(path);
        fs.attr(handle, attr.value());
        fs.close(handle);

        return 0;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int os_chown(lua_State* L) {
    int n = lua_gettop(L);

    expect_arg(L, 2, false);
    std::string path = get_string(L, 1);
    uint16_t uid = get_number(L, 2);
    uint16_t gid = n == 3 ? get_number(L, 3) : 0;

    try {
        auto& proc = os().executor->current(); 
        auto& fs = os().os->fs();
        auto exists = fs.exists(path);
        if (!exists) throw std::exception("not found");
        auto data = fs.filedata(path);

        if (data.fs_uid != proc.realuser() && proc.realuser() != 0) throw std::exception("no permission");

        auto handle = fs.open(path);
        fs.user(handle, uid);
        if (n == 3) {
            fs.group(handle, gid);
        }
        fs.close(handle);

        return 0;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int os_rm(lua_State* L) {
    expect_arg(L, 1);
    std::string path = get_string(L, 1);

    try {
        auto& proc = os().executor->current(); auto& fs = os().os->fs();
        auto exists = fs.exists(path);
        if (!exists) throw std::exception("not found");

        auto parent = fs::Path(path);
        parent.move_up();
        if (!os().os->can(parent.get(), proc.realuser(), proc.groups(), PERM_WRITE)) throw std::exception("no permission");

        fs.unlink(path);
        return 0;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int os_touch(lua_State* L) {
    expect_arg(L, 1);
    std::string path = get_string(L, 1);

    try {
        auto& proc = os().executor->current(); auto& fs = os().os->fs();
        auto exists = fs.is_file(path);
        if (!exists) throw std::exception("not found");

        if (!os().os->can(path, proc.realuser(), proc.groups(), PERM_WRITE)) throw std::exception("no permission");

        fs.touch(path);
        return 0;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int os_link(lua_State* L) {
    expect_arg(L, 2);
    std::string target = get_string(L, 1);
    std::string link = get_string(L, 2);

    try {
        auto& proc = os().executor->current(); 
        auto& fs = os().os->fs();
        if (!fs.is_file(target)) throw std::exception("target not found");
        if (fs.exists(link) || fs.is_catalog(link)) throw std::exception("invalid link value");

        auto parent1 = fs::Path(target);
        auto parent2 = fs::Path(link);
        parent1.move_up();
        parent2.move_up();

        if (!os().os->can(parent1.get(), proc.realuser(), proc.groups(), PERM_READ)) 
            throw std::exception("no permission");
        if (!os().os->can(parent2.get(), proc.realuser(), proc.groups(), PERM_WRITE)) 
            throw std::exception("no permission");

        fs.link(target, link);
        return 0;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int os_can(lua_State* L) {
    expect_arg(L, 2);
    auto path = get_string(L, 1);
    auto what = get_number(L, 2);

    try {
        auto& proc = os().executor->current();
        auto& fs = os().os->fs();

        auto exists = fs.exists(path);
        if (!exists) throw std::exception("not found");

        if (what < 0 || what > 2) throw std::exception("invalid argument");
        auto result = os().os->can(path, proc.realuser(), proc.groups(), what);
        lua_pushboolean(L, result);
        return 1;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int os_disk(lua_State* L) {
    expect_arg(L, 0);

    try {
        auto& fs = os().os->fs();
        auto& fsc = fs.core();
        auto& sb = fsc.sblock();
        auto& drive = fsc.drive();

        lua_newtable(L);
        luaL_insert_table_number(L, "dsec", drive.options().v_sec);
        luaL_insert_table_number(L, "dsize", drive.options().v_size);
        luaL_insert_table_number(L, "bsize", sb.s_bsize);
        luaL_insert_table_number(L, "fsize", sb.s_fsize);
        luaL_insert_table_number(L, "isize", sb.s_isize);
        luaL_insert_table_number(L, "asize", sb.s_asize);
        luaL_insert_table_number(L, "tinode", sb.s_tinode);
        luaL_insert_table_number(L, "tfree", sb.s_tfree);
        
        return 1;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int os_sleep(lua_State* L) {
    expect_arg(L, 1);
    int time = get_number(L, 1);

    try {
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
        return 0;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int tosopen_os_core(lua_State* L)
{
    luaL_Reg mylib[] = {
        {"exists", os_exists},
        {"is_file", os_isfile},
        {"is_catalog", os_iscatalog},
        {"listdir", os_listdir},
        {"mkdir", os_mkdir},
        {"clear", os_clear},
        {"filedata", os_filedata},
        {"chmod", os_chmod},
        {"chown", os_chown},
        {"rm", os_rm},
        {"touch", os_touch},
        {"link", os_link},
        {"can", os_can},
        {"disk", os_disk},
        {"sleep", os_sleep},
        {NULL, NULL},
    };
    luaL_newlib(L, mylib);
    return 1;
}
