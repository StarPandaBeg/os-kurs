#pragma once

#pragma comment(lib,"lua54.lib")
extern "C"
{
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
}
#include "bootstrap.h"

namespace tos {
	namespace searcher {
		namespace internal {
			void setup(const tos_global& os);
			const tos_global& os();
		}
	}
}