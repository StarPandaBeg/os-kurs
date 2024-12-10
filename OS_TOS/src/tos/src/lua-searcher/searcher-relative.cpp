#include "lua-searcher/searcher-relative.h"
#include "path.h"

using namespace tos::searcher::internal;

int searcher_relative(lua_State* L) {
	std::string name = luaL_checkstring(L, 1);

	lua_getglobal(L, "TOS_EXEC");

	if (!lua_isstring(L, -1)) {
		lua_pop(L, 1);
		lua_pushnil(L);
		return 1;
	}

	fs::Path path = lua_tostring(L, -1);
	path.move_up();
	path.add_component(name + ".lua");

	lua_pop(L, 1);
	
	auto& proc = os().executor->current();
	auto& fs = os().os->fs();
	auto exists = fs.is_file(path);
	if (!exists) {
		lua_pushnil(L);
		return 1;
	}
	if (!os().os->can(path.get(), proc.realuser(), proc.groups(), PERM_READ)) {
		throw std::exception("no permission");
	}

	auto handle = fs.open(path);
	fs.seek(handle, 0, FS_SEEK_END);
	auto fsize = fs.tell(handle);
	fs.seek(handle, 0);

	std::vector<char> buffer(fsize + 1);
	fs.read(handle, buffer.data(), fsize);
	fs.close(handle);

	luaL_loadstring(L, buffer.data());
	return 1;
}

lua_CFunction tos::searcher::tossearcher_relative(const tos_global& os)
{
	setup(os);
	return searcher_relative;
}
