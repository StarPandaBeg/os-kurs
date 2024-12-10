#include "tos.h"

#include <unordered_map>
#include <set>
#include <string>
#include "attr.h"

using namespace tos;
using namespace fs;

TOS::TOS(std::unique_ptr<Filesystem> fs) : m_fs(std::move(fs))
{
}

bool TOS::can(std::string path, uint16_t uid, std::set<uint16_t>& gids, uint8_t what)
{
	auto handle = m_fs->open(path);
	auto& fdata = m_fs->filedata(handle);
	m_fs->close(handle);

	FileAttr attr(fdata.fs_attr);
	if (fdata.fs_uid == uid) {
		return attr.can(0, what);
	}
	for (auto gid : gids) {
		if (fdata.fs_gid == gid) {
			return attr.can(1, what);
		}
	}
	return attr.can(2, what);
}

Filesystem& TOS::fs()
{
	return *m_fs.get();
}

TOS::filemode tos::TOS::to_mode(std::string mode_s)
{
	static std::set<std::string> const modes = {"r", "w", "a", "r+", "w+", "a+"};

	if (modes.count(mode_s) == 0) throw std::exception("invalid open mode");

	auto exist = mode_s[0] == 'r';
	auto append = mode_s[0] == 'a';
	auto read = exist || mode_s == "w+" || mode_s == "a+";
	auto write = mode_s[0] == 'w' || mode_s == "r+" || append;
	auto extend = mode_s[0] == 'w' || mode_s[0] == 'a';
	return TOS::filemode{ exist, read, write, extend, append };
}

uint32_t TOS::open(std::string path, std::string mode_s)
{
	auto mode = to_mode(mode_s);
	auto exists = m_fs->is_file(path);
	if (mode.exist && !exists) throw std::exception("file not found");

	auto handle = m_fs->open(path);
	m_files.emplace(std::make_pair(handle, mode_s));

	if (!mode.exist && exists && !mode.append) {
		m_fs->clear(handle);
	}
	if (mode.append) {
		m_fs->seek(handle, 0, FS_SEEK_END);
	}
	
	return handle;
}

void TOS::close(uint32_t handle)
{
	m_files.erase(handle);
	m_fs->close(handle);
}

uint32_t TOS::read(uint32_t handle, char* data, uint32_t count)
{
	auto it = m_files.find(handle);
	if (it == m_files.end()) throw std::exception("file not found");
	auto mode = to_mode(it->second);

	if (!mode.read) throw std::exception("no permission");
	return m_fs->read(handle, data, count);
}

void TOS::write(uint32_t handle, const char* data, uint32_t count)
{
	auto it = m_files.find(handle);
	if (it == m_files.end()) throw std::exception("file not found");
	auto mode = to_mode(it->second);

	if (!mode.write) throw std::exception("no permission");
	if (!mode.extend) {
		auto ptr = m_fs->tell(handle);
		auto data = m_fs->filedata(handle);
		if (ptr + count > data.fs_size) throw std::exception("no permission");
	}
	return m_fs->write(handle, data, count);
}

void TOS::seek(uint32_t handle, uint32_t count, int origin)
{
	m_fs->seek(handle, count, origin);
}

uint32_t TOS::tell(uint32_t handle)
{
	return m_fs->tell(handle);
}

uint32_t TOS::end(uint32_t handle)
{
	return m_fs->end(handle);
}
