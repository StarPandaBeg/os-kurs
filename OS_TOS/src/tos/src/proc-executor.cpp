#include "proc-executor.h"

#include <iostream>
#include <vector>

using namespace tos;

std::string process_error(const std::string& input);

ProcessExecutor::ProcessExecutor(TOS* tos) : m_os(tos)
{
}

uint16_t ProcessExecutor::suspawn(std::string path, uint16_t uid, uint16_t euid)
{
	auto& fs = m_os->fs();
	if (!fs.is_file(path)) throw std::exception("File not found");
	return m_handler.suspawn(path, uid, euid);
}

uint16_t ProcessExecutor::spawn(std::string path)
{
	auto parent = m_handler.current();
	return suspawn(path, parent.user(), parent.realuser());
}

void ProcessExecutor::execute(uint16_t pid)
{
	auto& parent = m_handler.current();
	m_handler.execute(pid);

	auto& proc = m_handler.current();
	auto& fs = m_os->fs();

	if (!fs.is_file(proc.path())) {
		m_handler.kill();
		throw std::exception("File not found");
	}

	set_env(proc);

	auto handle = fs.open(proc.path());
	fs.seek(handle, 0, FS_SEEK_END);
	auto fsize = fs.tell(handle);
	fs.seek(handle, 0);

	std::vector<char> buffer(fsize + 1); // +1 to force \0 as last symbol
	fs.read(handle, buffer.data(), buffer.size() - 1);
	fs.close(handle);

	int state = luaL_dostring(proc.state(), buffer.data());
	if (state != 0) {
		std::string error = lua_tostring(proc.state(), 1);
		error = process_error(error);
		
		kill();
		throw std::exception(error.c_str());
	}
	kill();
}

Process& ProcessExecutor::get(uint16_t pid)
{
	return m_handler.get(pid);
}

Process& ProcessExecutor::current()
{
	return m_handler.current();
}

uint16_t ProcessExecutor::current_pid()
{
	return m_handler.current_pid();
}

void ProcessExecutor::kill()
{
	auto& proc = m_handler.current();
	auto& handle = proc.handle();

	for (auto& h : handle) {
		m_os->close(h);
	}
	m_handler.kill();
}

void ProcessExecutor::set_env(Process& proc)
{
	auto L = proc.state();
	lua_newtable(L);
	for (auto pair : proc.env()) {
		lua_pushstring(L, pair.first.c_str());
		lua_pushstring(L, pair.second.c_str());
		lua_settable(L, -3);
	}
	lua_setglobal(L, "TOS_ENV");

	lua_pushstring(L, proc.path().c_str());
	lua_setglobal(L, "TOS_EXEC");
}

std::string process_error(const std::string& input) {
	size_t firstColonPos = input.find(':');
	if (firstColonPos == std::string::npos) {
		return input;
	}

	size_t secondColonPos = input.find(':', firstColonPos + 1);
	if (secondColonPos == std::string::npos) {
		return input;
	}
	return input.substr(secondColonPos + 2); // +1 for space
}