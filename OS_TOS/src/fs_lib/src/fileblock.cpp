#include "fileblock.h"

#include <iostream>
#include <vector>

using namespace fs;
using namespace fs::internal;

template <class T>
T powsum(T base, T level) {
	if (level == 0) return 0;
	T result = 0;
	for (T i = 1; i <= level; i++) {
		result += (T)std::pow(base, i);
	}
	return result;
}

std::pair<uint32_t, uint32_t> level_borders(uint32_t perblock, uint8_t level) {
	uint32_t left = 7 + powsum(perblock, (uint32_t)level - 1);
	uint32_t right = left + std::pow(perblock, level) - 1;
	return std::make_pair(left, right);
}

FileblockAllocator::FileblockAllocator(VirtualDrive& vdrive, const fballoc_options& opts) : m_vdrive(vdrive), m_allocate(opts.allocate), m_clear(opts.clear), m_free(opts.free)
{
	auto vopts = vdrive.options();
	m_bsize = vopts.v_bsize;
	m_acount = m_bsize / sizeof(uint32_t);
}

void FileblockAllocator::resize(inode& inode, uint32_t size, bool clear)
{
	auto current_size = inode.i_fsize;
	if (size == current_size) return;
	
	if (size > current_size) {
		extend(inode, size - current_size, clear);
	}
	else {
		shrink(inode, current_size - size);
	}
}

void FileblockAllocator::extend(inode& inode, uint32_t count, bool clear)
{
	auto index = inode.i_fsize;
	for (uint32_t i = 0; i < count; i++) {
		auto address = m_allocate();
		if (clear) m_clear(address);
		set_cluster(inode, index + i, address);
	}
}

void FileblockAllocator::shrink(inode& inode, uint32_t count)
{
	auto index = inode.i_fsize;
	for (uint32_t i = 0; i < count; i++) {
		free_cluster(inode, index - i - 1);
	}
}

void FileblockAllocator::set_cluster(inode& inode, uint32_t index, uint32_t address)
{
	auto level = get_indirection_level(index);
	set_indirection_cluster(inode, index, level, address);
}

uint32_t FileblockAllocator::get_cluster(inode& inode, uint32_t index)
{
	auto level = get_indirection_level(index);
	return get_indirection_cluster(inode, index, level);
}

uint8_t FileblockAllocator::get_indirection_level(uint32_t index)
{
	if (index < INODE_DIRECT_ADDRESSABLE) return 0;
	auto adj_index = index - INODE_DIRECT_ADDRESSABLE;
	
	uint32_t total = m_acount;
	for (int i = 1; i < 5; i++) {
		if (adj_index < total) return i;
		total += (uint32_t)pow(m_acount, i+1);
	}
	throw std::exception("max indirection level reached");
}

void FileblockAllocator::set_indirection_cluster(inode& inode, uint32_t rindex, uint8_t level, uint32_t address)
{
	if (level == 0) {
		inode.i_fsize = std::max(rindex + 1, inode.i_fsize);
		inode.i_blocks[rindex] = address;
		return;
	}

	auto bindex = INODE_DIRECT_ADDRESSABLE + level - 1;
	auto borders = level_borders(m_acount, level);
	auto baddress = inode.i_blocks[bindex];

	auto fsize = inode.i_fsize;
	if (fsize <= borders.first) {
		baddress = m_allocate();
		m_clear(baddress);
		inode.i_blocks[bindex] = baddress;
	}
	set_indirection_cluster_address(inode, rindex, level, address, baddress);
}

uint32_t FileblockAllocator::set_indirection_cluster_address(inode& inode, uint32_t rindex, uint8_t level, uint32_t address, uint32_t baddress)
{
	static std::vector<char> buffer(m_bsize);

	if (level == 0) throw std::exception("invalid indirection level");

	uint16_t c1 = level == 1 ? 0 : level - 2;

	auto borders = level_borders(m_acount, level);
	auto bsize = (level == 1) ? 1 : m_acount;       // size of addressed block
	auto index = (rindex - borders.first) / bsize;  // index of address inside current block
	auto cindex = (rindex - borders.first) % bsize + INODE_DIRECT_ADDRESSABLE + powsum(m_acount, c1); // index of address inside child block

	auto caddress = address;
	if (level != 1) {
		m_vdrive.seek(baddress);
		m_vdrive.read(buffer.data());
		memcpy(&caddress, buffer.data() + index * sizeof(caddress), sizeof(caddress));

		// ТУТ БЕДА - понять нельзя что блок выделен
		if (caddress == 0) {
			caddress = m_allocate();
			m_clear(caddress);
		}
		caddress = set_indirection_cluster_address(inode, cindex, level - 1, address, caddress);
	}

	m_vdrive.seek(baddress);
	m_vdrive.read(buffer.data());
	memcpy(buffer.data() + index * sizeof(caddress), &caddress, sizeof(caddress));
	m_vdrive.seek(baddress);
	m_vdrive.write(buffer.data());

	inode.i_fsize = std::max(rindex + 1, inode.i_fsize);
	return baddress;
}

uint32_t FileblockAllocator::get_indirection_cluster(inode& inode, uint32_t rindex, uint8_t level)
{
	if (rindex >= inode.i_fsize) 
		throw std::exception("invalid cluster index");
	if (level == 0) return inode.i_blocks[rindex];

	auto bindex = INODE_DIRECT_ADDRESSABLE + level - 1;
	auto baddress = inode.i_blocks[bindex];
	return get_indirection_cluster(inode, rindex, level, baddress);
}

uint32_t FileblockAllocator::get_indirection_cluster(inode& inode, uint32_t rindex, uint8_t level, uint32_t baddress)
{
	static std::vector<char> buffer(m_bsize);

	if (level == 0) throw std::exception("invalid indirection level");

	uint16_t c1 = level == 1 ? 0 : level - 2;

	auto borders = level_borders(m_acount, level);
	auto bsize = (level == 1) ? 1 : m_acount;       // size of addressed block
	auto index = (rindex - borders.first) / bsize;  // index of address inside current block
	auto cindex = (rindex - borders.first) % bsize + INODE_DIRECT_ADDRESSABLE + powsum(m_acount, c1); // index of address inside child block
	
	uint32_t caddress = 0;

	m_vdrive.seek(baddress);
	m_vdrive.read(buffer.data());
	memcpy(&caddress, buffer.data() + index * sizeof(caddress), sizeof(caddress));

	if (level == 1) return caddress;
	return get_indirection_cluster(inode, cindex, level - 1, caddress);
}

void FileblockAllocator::free_cluster(inode& inode, uint32_t index)
{
	auto level = get_indirection_level(index);
	free_indirection_cluster(inode, index, level);
}

void FileblockAllocator::free_indirection_cluster(inode& inode, uint32_t rindex, uint8_t level)
{
	if (level == 0) {
		inode.i_fsize--;
		m_clear(inode.i_blocks[rindex]); // TMP
		m_free(inode.i_blocks[rindex]);
		inode.i_blocks[rindex] = 0;
		return;
	}

	auto bindex = INODE_DIRECT_ADDRESSABLE + level - 1;
	auto baddress = inode.i_blocks[bindex];

	free_indirection_cluster(inode, rindex, level, baddress);
}

void FileblockAllocator::free_indirection_cluster(inode& inode, uint32_t rindex, uint8_t level, uint32_t baddress)
{
	static std::vector<char> buffer(m_bsize);

	if (level == 0) throw std::exception("invalid indirection level");

	uint16_t c1 = level == 1 ? 0 : level - 2;

	auto borders = level_borders(m_acount, level);
	auto bsize = (level == 1) ? 1 : m_acount;       // size of addressed block
	auto index = (rindex - borders.first) / bsize;  // index of address inside current block
	auto cindex = (rindex - borders.first) % bsize + INODE_DIRECT_ADDRESSABLE + powsum(m_acount, c1); // index of address inside child block

	uint32_t caddress = 0;

	m_vdrive.seek(baddress);
	m_vdrive.read(buffer.data());
	memcpy(&caddress, buffer.data() + index * sizeof(caddress), sizeof(caddress));

	if (level == 1) {
		m_free(caddress);
		inode.i_fsize--;
	}
	else {
		free_indirection_cluster(inode, cindex, level - 1, caddress);
	}

	if (rindex <= borders.first) {
		m_free(baddress);
	}
}
