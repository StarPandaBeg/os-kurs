#include "lua-lib/lib-crypto.h"
#include "hash/sha-256.h"

using namespace tos;
using namespace tos::lib;
using namespace tos::lib::internal;

int tosopen_crypto_core(lua_State* L);

lua_CFunction tos::lib::tosopen_crypto(const tos_global& os)
{
    setup(os);
    return tosopen_crypto_core;
}

int crypto_sha256(lua_State* L) {
    expect_arg(L, 1);
    std::string data = get_string(L, 1);

    try {
        SHA256 sha;
        sha.update(data);
        std::array<uint8_t, 32> digest = sha.digest();
        lua_pushstring(L, SHA256::toString(digest).c_str());
        return 1;
    }
    catch (std::exception& e) {
        luaL_error(L, e.what());
    }
    return 0;
}

int tosopen_crypto_core(lua_State* L)
{
    luaL_Reg mylib[] = {
        {"sha256", crypto_sha256},
        {NULL, NULL}
    };
    luaL_newlib(L, mylib);
    return 1;
}
