#pragma once

#include <cstdint>
#include "proc-handler.h"
#include "lua-registry.h"
#include "tos.h"

namespace tos {
	class ProcessExecutor {
	public:
		ProcessExecutor(TOS* tos);

		uint16_t suspawn(std::string path, uint16_t uid, uint16_t euid);
		uint16_t spawn(std::string path);
		void execute(uint16_t pid);

		Process& get(uint16_t pid);
		Process& current();
		uint16_t current_pid();
	private:
		TOS* m_os;
		ProcessHandler m_handler;

		void kill();
		void set_env(Process& proc);
	};
}