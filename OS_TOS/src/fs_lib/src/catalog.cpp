#include "catalog.h"

#include <vector>

using namespace fs;
using namespace fs::internal;

catalog_record to_catalog_record(catalog_entry& entry) {
	return catalog_record{
		entry,
		false
	};
}

CatalogHandle::CatalogHandle(FilesystemCore& fcore, const uint32_t index) : m_fcore(fcore), m_index(index)
{
	scan();
}

void CatalogHandle::add(uint32_t inode, std::string filename)
{
	if (inode == m_index) {
		throw std::invalid_argument("Cannot create link to itself");
	}
	/*if (m_cache.count(inode) > 0) {
		throw std::runtime_error("Duplicate inode found");
	}*/

	if (filename.empty()) {
		throw std::invalid_argument("Filename cannot be empty");
	}
	if (filename.size() >= sizeof(catalog_entry::c_name)) {
		throw std::invalid_argument("Filename is too long");
	}
	if (exists(filename)) {
		throw std::runtime_error("Duplicate filename found");
	}

	auto entry = catalog_entry{inode};
	memcpy(entry.c_name, filename.data(), sizeof(catalog_entry::c_name) - 1);
	entry.c_name[sizeof(catalog_entry::c_name) - 1] = '\0';

	auto record = to_catalog_record(entry);
	m_cache2.emplace(inode, filename);
	m_cache3[filename] = std::move(record);
	// m_cache.insert(std::make_pair(inode, std::move(record)));
}

bool CatalogHandle::exists(std::string filename)
{
	return m_cache3.count(filename) > 0;
}

int CatalogHandle::remove(uint32_t index)
{
	auto totalRemoved = 0;
	for (auto [it, rangeEnd] = m_cache2.equal_range(index); it != rangeEnd; ++it) {
		auto& fname = it->second;
		m_cache3.erase(fname);
		totalRemoved++;
	}
	m_cache2.erase(index);
	return totalRemoved;
}

int CatalogHandle::remove(std::string filename)
{
	auto index = get(filename);
	m_cache2.erase(index);
	m_cache3.erase(filename);
	return 1;
}

uint32_t CatalogHandle::get(std::string filename)
{
	if (!exists(filename)) throw std::runtime_error("Entry not found");
	return m_cache3[filename].c_entry.c_inode;	
}

std::string CatalogHandle::get(uint32_t inode)
{
	auto it = m_cache3.begin();
	catalog_record* record = nullptr;
	for (int i = 0; it != m_cache3.end(); it++, i++) {
		record = &it->second;
		if (record->c_entry.c_inode == inode) return it->first;
	}
	throw std::runtime_error("Entry not found");
}

const std::vector<catalog_entry> CatalogHandle::list()
{
	std::vector<catalog_entry> result;
	result.reserve(m_cache3.size());
	for (const auto& pair : m_cache3) {
		result.push_back(pair.second.c_entry);
	}
	return result;
}

uint32_t CatalogHandle::index() const
{
	return m_index;
}

void CatalogHandle::close()
{
	m_fcore.seek(m_index, 0);
	m_fcore.resize_inode(m_index, m_cache3.size() * sizeof(catalog_entry));

	std::vector<char> buffer(sizeof(catalog_entry), 0);
	auto it = m_cache3.begin();
	for (int i = 0; it != m_cache3.end(); it++, i++) {
		auto& record = it->second;
		memcpy(buffer.data(), &record.c_entry, sizeof(catalog_entry));
		m_fcore.write(m_index, buffer.data(), buffer.size());
	}
}

void CatalogHandle::scan()
{
	auto total = count(false);
	std::vector<char> buffer(sizeof(catalog_entry) * total, 0);

	m_fcore.seek(m_index, 0);
	m_fcore.read(m_index, buffer.data(), buffer.size());

	catalog_entry entry;
	for (uint32_t i = 0; i < total; i++) {
		memcpy(&entry, buffer.data() + i * sizeof(catalog_entry), sizeof(catalog_entry));
		auto record = to_catalog_record(entry);
		std::string name = record.c_entry.c_name;
		m_cache2.emplace(record.c_entry.c_inode, name);
		m_cache3.emplace(name, record);
	}
}

uint32_t CatalogHandle::count(bool cached)
{
	if (cached) {
		return m_cache3.size();
	}
	auto data = m_fcore.get_inode(m_index);
	return data.fs_size / sizeof(catalog_entry);
}
