#include "filesystem.h"
#include "attr.h"

#include <algorithm>
#include <iostream>

using namespace fs;
using namespace fs::internal;

#define DIR_ATTR_DEFAULT 0b1100000111101101       // drwxr-xr-x
#define FILE_ATTR_DEFAULT 0b0100000110100000      // -rw-r-----

void Filesystem::format(std::shared_ptr<Drive> drive, const fs_format_options& options)
{
	FilesystemCore::format(drive, options);
	Filesystem fs(drive);

	auto root_dir_index = fs.create_catalog(fs_inode_options{ DIR_ATTR_DEFAULT, 0, 0 });
	// todo: increase links count
	fs.close_catalog(root_dir_index);
}

Filesystem::Filesystem(std::shared_ptr<Drive> drive) : m_fcore(FilesystemCore(drive))
{
	clear_tree();
}

uint32_t Filesystem::open(Path path)
{
	if (path.level() == 0) {
		return m_fcore.open_inode(0);
	}

	auto parent_path = path.get_level(path.level() - 1);
	auto pindex = find_index(parent_path);
	uint32_t index;
	bool was_created = false;

	auto pchandle = open_catalog(pindex);
	if (pchandle.exists(path.filename())) {
		/*if (is_catalog(path)) {
			close_catalog(pchandle);
			throw std::runtime_error("Cannot open catalog as file");
		}*/

		index = pchandle.get(path.filename());
		index = m_fcore.open_inode(index);
	}
	else {
		index = m_fcore.create_inode({FILE_ATTR_DEFAULT, 0, 0});
		was_created = true;
	}

	close_catalog(pchandle);
	if (was_created) link(index, path);
	return index;
}

void Filesystem::close(uint32_t index)
{
	m_fcore.close_inode(index);
}

uint32_t Filesystem::read(uint32_t index, char* data, uint32_t count)
{
	return m_fcore.read(index, data, count);
}

void Filesystem::write(uint32_t index, const char* data, uint32_t count)
{
	m_fcore.write(index, data, count);
}

void Filesystem::seek(uint32_t index, uint32_t count, int origin)
{
	m_fcore.seek(index, count, origin);
}

uint32_t Filesystem::end(uint32_t index)
{
	return m_fcore.end(index);
}

uint32_t Filesystem::tell(uint32_t index)
{
	return m_fcore.tell(index);
}

void Filesystem::clear(uint32_t index)
{
	m_fcore.resize_inode(index, 0, true);
}

void Filesystem::touch(uint32_t index)
{
	m_fcore.touch_inode(index);
}

bool Filesystem::is_open(uint32_t index)
{
	return m_fcore.is_open(index);
}

const fs_inode_data Filesystem::filedata(uint32_t index)
{
	return m_fcore.get_inode(index);
}

void Filesystem::attr(uint32_t index, uint16_t attr)
{
	auto data = filedata(index);

	fs_inode_options options;
	memcpy(&options, &data, sizeof(fs_inode_options));

	options.fs_attr = attr;
	m_fcore.set_inode(index, options);
}

void Filesystem::user(uint32_t index, uint16_t uid)
{
	auto data = filedata(index);

	fs_inode_options options;
	memcpy(&options, &data, sizeof(fs_inode_options));

	options.fs_uid = uid;
	m_fcore.set_inode(index, options);
}

void Filesystem::group(uint32_t index, uint16_t gid)
{
	auto data = filedata(index);

	fs_inode_options options;
	memcpy(&options, &data, sizeof(fs_inode_options));

	options.fs_gid = gid;
	m_fcore.set_inode(index, options);
}

void Filesystem::create_catalog(Path path, uint16_t owner, uint16_t group)
{
	if (exists(path)) {
		throw std::runtime_error("Duplicate filename found");
	}

	auto parent_path = path.get_level(path.level() - 1);
	auto index = find_index(parent_path);
	auto pchandle = open_catalog(index);

	auto chandle = create_catalog(fs_inode_options{ DIR_ATTR_DEFAULT, owner, group });
	auto cindex = chandle.index();
	close_catalog(chandle);

	pchandle.add(cindex, path.filename());
	close_catalog(pchandle);
}

const std::vector<std::string> Filesystem::list_catalog(Path path)
{
	auto index = find_index(path);
	if (is_file(path)) {
		throw std::runtime_error("Cannot open file as catalog");
	}
	auto chandle = open_catalog(index);
	auto files = chandle.list();
	close_catalog(chandle);

	std::vector<std::string> result;
	result.reserve(files.size());
	std::transform(files.begin(), files.end(), std::back_inserter(result),
		[](const catalog_entry& s) {
			return s.c_name;
		}
	);
	return result;
}

void Filesystem::remove(Path path)
{
	auto parent_path = path.get_level(path.level() - 1);
	auto pindex = find_index(parent_path);
	auto index = find_index(path);

	remove(index, pindex);
	clear_tree();
}

bool Filesystem::is_file(Path path)
{
	if (!exists(path)) return false;

	auto index = find_index(path);
	return is_file(index);
}

bool Filesystem::is_catalog(Path path)
{
	if (!exists(path)) return false;
	return !is_file(path);
}

bool Filesystem::exists(Path path)
{
	try {
		find_index(path);
		return true;
	}
	catch (std::exception& e) {
		return false;
	}
}

void Filesystem::touch(Path path)
{
	auto index = find_index(path);
	auto handle = m_fcore.open_inode(index);
	m_fcore.touch_inode(handle);
	m_fcore.close_inode(handle);
}

void Filesystem::link(Path path, Path link)
{
	if (!is_file(path)) throw std::exception("Cannot create link to non a file entry");
	auto sourceIndex = find_index(path);
	auto handle = m_fcore.open_inode(sourceIndex);
	this->link(handle, link);
	m_fcore.close_inode(handle);
}

void Filesystem::unlink(Path path)
{
	if (!is_file(path)) throw std::exception("Broken link");
	auto filename = path.filename();
	auto index = find_index(path);
	auto handle = m_fcore.open_inode(index);

	this->unlink(index, path);

	auto data = m_fcore.get_inode(handle);
	m_fcore.close_inode(handle);

	if (data.fs_links == 0) {
		auto pindex = find_index(path.get_level(path.level() - 1));
		remove(index, pindex);
		clear_tree();
	}
}

const fs_inode_data Filesystem::filedata(Path path)
{
	auto index = find_index(path);
	auto handle = m_fcore.open_inode(index);
	auto fdata = m_fcore.get_inode(handle);
	m_fcore.close_inode(handle);
	return fdata;
}

::FilesystemCore& fs::Filesystem::core()
{
	return m_fcore;
}

CatalogHandle Filesystem::create_catalog(const fs_inode_options& options)
{
	fs_inode_options options_ = options;
	FileAttr attr(options_.fs_attr);
	options_.fs_attr = attr.is_dir(true).value();
	
	auto index = m_fcore.create_inode(options_);
	m_fcore.touch_inode(index);
	return CatalogHandle(m_fcore, index);
}

CatalogHandle Filesystem::open_catalog(uint32_t index)
{
	auto index_ = m_fcore.open_inode(index);
	auto attr = FileAttr(m_fcore.get_inode(index_).fs_attr);
	if (attr.is_file()) {
		m_fcore.close_inode(index_);
		throw std::runtime_error("Cannot open file as catalog");
	}
	return CatalogHandle(m_fcore, index_);
}

void Filesystem::close_catalog(internal::CatalogHandle& index)
{
	index.close();
	m_fcore.close_inode(index.index());
}

bool Filesystem::is_file(uint32_t index)
{
	if (!exists(index)) return false;
	if (m_fcore.is_open(index)) {
		auto data = m_fcore.get_inode(index);
		return FileAttr(data.fs_attr).is_file();
	}
	auto inode = m_fcore.open_inode(index);
	auto data = m_fcore.get_inode(inode);
	m_fcore.close_inode(inode);
	return FileAttr(data.fs_attr).is_file();
}

bool Filesystem::is_catalog(uint32_t index)
{
	if (!exists(index)) return false;
	return !is_file(index);
}

bool Filesystem::exists(uint32_t index)
{
	try {
		if (m_fcore.is_open(index)) return true;
		auto handle = m_fcore.open_inode(index);
		m_fcore.close_inode(handle);
		return true;
	}
	catch (std::exception& e) {
		return false;
	}
}

/// @deprecated - Use unlink instead
void Filesystem::remove(uint32_t index, uint32_t pindex)
{
	if (is_catalog(index)) {
		auto handle = open_catalog(index);
		auto items = handle.list();
		close_catalog(handle);

		for (auto& item : items) {
			remove(item.c_inode, index);
		}
	}

	// Skip parent for root inode index
	if (index != 0) {
		auto phandle = open_catalog(pindex);
		phandle.remove(index);
		close_catalog(phandle);
	}

	m_fcore.free_inode(index);
}

void Filesystem::link(uint32_t index, Path link)
{
	if (exists(link) || is_catalog(link)) throw std::exception("Invalid link");

	auto filename = link.filename();
	link.move_up();

	auto targetIndex = find_index(link);
	auto chandle = open_catalog(targetIndex);
	chandle.add(index, filename);
	close_catalog(chandle);

	m_fcore.link_inode(index);
}

void Filesystem::unlink(uint32_t index, Path link)
{
	if (!exists(link) || is_catalog(link)) throw std::exception("broken link");

	auto filename = link.filename();
	link.move_up();

	auto targetIndex = find_index(link);
	auto chandle = open_catalog(targetIndex);
	chandle.remove(filename);
	close_catalog(chandle);

	m_fcore.unlink_inode(index);
}

uint32_t Filesystem::find_index(Path path)
{
	uint32_t index;
	if (m_tree.search(path, index)) {
		return index;
	}

	auto level = find_cached_level(path, index);
	auto target = path.level();
	Path tmp_path = path.get_level(level + 1);

	for (int i = level; i < target; i++) {
		if (i != level) {
			tmp_path.add_component(path.filename_level(i + 1));
		}

		auto chandle = open_catalog(index);
		if (!chandle.exists(tmp_path.filename())) {
			close_catalog(chandle);
			throw std::runtime_error("Entry not found");
		}
		index = chandle.get(tmp_path.filename());
		close_catalog(chandle);

		m_tree.insert(tmp_path, index);
	}
	return index;
}

int Filesystem::find_cached_level(Path path, uint32_t& address)
{
	while (path.level() > 0) {
		if (m_tree.search(path, address)) {
			return path.level();
		}
		path.move_up();
	}
	address = 0;
	return 0;
}

void Filesystem::clear_tree()
{
	m_tree.clear();
	m_tree.insert("/", 0);
}
