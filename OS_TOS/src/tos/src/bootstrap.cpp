#include "bootstrap.h"
#include "lua-registry.h"
#include "lua-lib/lib-crypto.h"
#include "lua-lib/lib-os.h"
#include "lua-lib/lib-io.h"
#include "lua-lib/lib-proc.h"
#include "lua-searcher/searcher-file.h"
#include "lua-searcher/searcher-module.h"
#include "lua-searcher/searcher-relative.h"

#include <iostream>

using namespace tos;

void register_libs();
void register_searcher();

tos_global* os = nullptr;

tos_global bootstrap(const tos_config& config)
{
	auto drive = std::make_shared<fs::Drive>(config.path);
	auto fs = std::make_unique<fs::Filesystem>(drive);

	auto tos = new TOS(std::move(fs));
	auto executor = new ProcessExecutor(tos);

	os = new tos_global{ tos, executor };
	register_libs();
	register_searcher();
	return *os;
}

void shutdown()
{
	delete os->executor;
	delete os->os;
	delete os;
}

void register_libs()
{
	auto& reg = LuaRegistry::instance();

	reg.register_lib("crypto", lib::tosopen_crypto(*os));
	reg.register_lib("io-raw", lib::tosopen_io(*os));
	reg.register_lib("os-raw", lib::tosopen_os(*os));
	reg.register_lib("proc", lib::tosopen_proc(*os));
}

void register_searcher()
{
	auto& reg = tos::LuaRegistry::instance();
	reg.register_searcher(searcher::tossearcher_file(*os));
	reg.register_searcher(searcher::tossearcher_module(*os));
	reg.register_searcher(searcher::tossearcher_relative(*os));
}
