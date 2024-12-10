#include "proc-handler.h"

#include <random>
#include "lua-registry.h"

using namespace tos;

uint16_t new_pid()
{
	static std::random_device dev;
	static std::mt19937 rng(dev());
	static std::uniform_int_distribution<std::mt19937::result_type> dist(0, 65535);
	return dist(rng);
}

ProcessHandler::ProcessHandler()
{
	auto system_proc = spawn("", 0, 0);
	m_running.push(system_proc);
}

uint16_t ProcessHandler::spawn(std::string path)
{
	auto parent = m_running.front();
	return suspawn(path, parent.p.user(), parent.p.realuser());
}

uint16_t ProcessHandler::suspawn(std::string path, uint16_t uid, uint16_t euid)
{
	if (m_running.size() >= 256) throw std::exception("too many processes");

	auto parent = m_running.front();
	auto proc = spawn(path, uid, euid);
	auto pid = proc.pid;
	m_ready.emplace(std::make_pair(pid, std::move(proc)));
	m_tree.emplace(parent.pid, pid);
	return proc.pid;
}

void ProcessHandler::execute(uint16_t pid)
{
	if (m_ready.count(pid) == 0) throw std::exception("invalid state");
	auto it = m_ready.find(pid);
	auto& proc = it->second;

	auto& registry = LuaRegistry::instance();
	auto state = registry.create_state_for(pid);
	proc.p.state(state);

	m_running.push(proc);
	m_ready.erase(it);
}

void ProcessHandler::kill()
{
	// todo: handle case when trying to kill last system proc
	auto proc = m_running.front();
	lua_close(proc.p.state());
	m_running.pop();
	m_tree.erase(proc.pid);
}

Process& ProcessHandler::current()
{
	return m_running.front().p;
}

uint16_t tos::ProcessHandler::current_pid()
{
	return m_running.front().pid;
}

Process& ProcessHandler::get(uint16_t pid)
{
	auto it = m_ready.find(pid);
	if (it != m_ready.end()) {
		return it->second.p;
	}
	auto entry = m_running.find([pid](const proc& p) {
		return p.pid == pid;
	});
	if (entry == nullptr) throw std::exception("process not found");
	return entry->p;
}

ProcessHandler::proc ProcessHandler::spawn(std::string path, uint16_t uid, uint16_t euid)
{
	auto p = Process(path);
	p.user(uid);
	p.realuser(euid);
	return proc{new_pid(), p};
}
