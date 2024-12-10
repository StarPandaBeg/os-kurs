#include "proc.h"

using namespace tos;

Process::Process() : Process("")
{
}

Process::Process(std::string path) : m_uid(0), m_gid(0), m_euid(0), m_path(path)
{
}

uint16_t Process::user() const
{
	return m_uid;
}

void Process::user(uint16_t value)
{
	m_uid = value;
}

uint16_t Process::group() const
{
	return m_gid;
}

void Process::group(uint16_t value)
{
	m_gid = value;
}

uint16_t Process::realuser() const
{
	return m_euid;
}

void Process::realuser(uint16_t value)
{
	m_euid = value;
}

const std::string& Process::path() const
{
	return m_path;
}

lua_State* Process::state() const
{
	return m_state;
}

void Process::state(lua_State* state)
{
	m_state = state;
}

std::set<uint32_t>& Process::handle()
{
	return m_handle;
}

std::set<uint16_t>& tos::Process::groups()
{
	return m_groups;
}

bool tos::Process::hasenv(std::string key)
{
	return m_env.count(key) > 0;
}

std::map<std::string, std::string>& Process::env()
{
	return m_env;
}

std::string Process::env(std::string key)
{
	return m_env[key];
}

void Process::env(std::string key, std::string value)
{
	m_env[key] = value;
}
