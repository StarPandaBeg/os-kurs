#pragma once

#include <map>

#include "proc.h"
#include "stack.h"

namespace tos {
	class ProcessHandler {
	public:
		ProcessHandler();

		uint16_t spawn(std::string path);
		uint16_t suspawn(std::string path, uint16_t uid, uint16_t euid);

		void execute(uint16_t pid);
		void kill();

		Process& get(uint16_t pid);
		Process& current();
		uint16_t current_pid();
	private:
		struct proc {
			uint16_t pid;
			Process p;
		};
		Stack<proc> m_running;
		std::map<uint16_t, proc> m_ready;
		std::multimap<uint16_t, uint16_t> m_tree;

		proc spawn(std::string path, uint16_t uid, uint16_t euid);
	};
}