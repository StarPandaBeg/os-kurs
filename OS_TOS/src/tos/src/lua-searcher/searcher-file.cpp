#include "lua-searcher/searcher-file.h"

using namespace tos::searcher::internal;

int searcher_file(lua_State* L) {
	std::string name = luaL_checkstring(L, 1);
	std::string path = name[0] == '/' ? (name + ".lua") : ("/lib/" + name + ".lua");

	auto& proc = os().executor->current();
	auto& fs = os().os->fs();
	auto exists = fs.is_file(path);
	if (!exists) {
		lua_pushnil(L);
		return 1;
	}
	if (!os().os->can(path, proc.realuser(), proc.groups(), PERM_READ)) {
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

lua_CFunction tos::searcher::tossearcher_file(const tos_global& os)
{
	setup(os);
	return searcher_file;
}
