#include "filesystem_core.h"
#include "attr.h"

#include <iostream>
#include <vector>

using namespace fs;
using namespace fs::internal;

superblock to_super_block(const drive_options& dopts, const fs_format_options& fopts) {
	if (fopts.fs_bsize % dopts.v_sec != 0 || dopts.v_sec > fopts.fs_bsize) {
		throw std::runtime_error("Invalid disk block size. It must be a multiple of the disk sector size.");
	}

	uint16_t bcoef = fopts.fs_bsize / dopts.v_sec;
	superblock sb;

	uint32_t inodes_per_block = fopts.fs_bsize / INODE_SIZE;
	uint32_t inodes_total = inodes_per_block * fopts.fs_isize;

	sb.s_magic = FS_MAGIC;
	sb.s_bsize = fopts.fs_bsize;
	sb.s_fsize = (dopts.v_size - 1) / bcoef; // Substract 1 sector for superblock itself
	sb.s_isize = fopts.fs_isize;
	sb.s_asize = (uint32_t)std::ceil(sb.s_fsize * CLUSTER_RECORD_SIZE / (float)sb.s_bsize);
	sb.s_tinode = inodes_total;
	sb.s_ttinode = std::min(inodes_total, (uint32_t)INODE_TABLE_SIZE);
	sb.s_tfree = sb.s_fsize - sb.s_isize - sb.s_asize;

	memset(sb.s_inode, 0, sizeof(sb.s_inode));
	for (uint32_t i = 0, j = sb.s_ttinode - 1; i < sb.s_ttinode; i++, j--) {
		sb.s_inode[i] = j;
	}
	return sb;
}

fs_inode_record to_inode_record(const inode inode) {
	return {
		inode,
		false,
		0
	};
}

void FilesystemCore::format(std::shared_ptr<Drive> drive, const fs_format_options& options)
{
	auto sb = to_super_block(drive->options(), options);
	write_superblock(drive, sb);

	VirtualDrive vdrive(drive, vdrive_options{ sb.s_bsize, 1, sb.s_fsize });
	ClusterTable ctable(vdrive, vpartition_options{ sb.s_bsize, sb.s_isize, sb.s_asize }, 0);

	std::vector<char> zerobuf(sb.s_bsize);
	for (uint32_t i = 0; i < sb.s_isize; i++) {
		vdrive.write(zerobuf.data());
	}

	auto busy_clusters = sb.s_isize + sb.s_asize;
	for (uint32_t i = sb.s_fsize - 1; i >= busy_clusters; i--) {
		ctable.push(i);
	}
}

void FilesystemCore::read_superblock(std::shared_ptr<Drive> drive, superblock& sb)
{
	std::vector<char> buffer(drive->options().v_sec, 0);
	drive->seek(0);
	drive->read(buffer.data());
	std::memcpy(&sb, buffer.data(), sizeof(sb));

	if (sb.s_magic != FS_MAGIC) {
		throw std::runtime_error("Unable to read superblock. Data may be corrupted.");
	}
}

void FilesystemCore::write_superblock(std::shared_ptr<Drive> drive, const superblock& sb)
{
	drive->seek(0);
	std::vector<char> buffer(drive->options().v_sec, 0);
	std::memcpy(buffer.data(), &sb, sizeof(sb));
	drive->write(buffer.data());
}

ICache::iterator FilesystemCore::get_cache_iterator(uint32_t index)
{
	auto it = m_icache.find(index);
	if (it == m_icache.end()) {
		throw std::runtime_error("File index not found.");
	}
	return it;
}

FilesystemCore::FilesystemCore(std::shared_ptr<Drive> drive) : m_drive(drive)
{
	read_superblock(m_drive, m_sblock);
	m_vdrive = std::make_unique<VirtualDrive>(m_drive, vdrive_options{m_sblock.s_bsize, 1, m_sblock.s_fsize});
	m_itable = std::make_unique<INodeTable>(*m_vdrive, vpartition_options{m_sblock.s_bsize, 0, m_sblock.s_isize});
	m_ctable = std::make_unique<ClusterTable>(*m_vdrive, vpartition_options{ m_sblock.s_bsize, m_sblock.s_isize, m_sblock.s_asize }, m_sblock.s_tfree);

	auto allocate = std::bind(&FilesystemCore::allocate_cluster, this);
	auto clear = std::bind(&FilesystemCore::clear_cluster, this, std::placeholders::_1);
	auto free = std::bind(&FilesystemCore::free_cluster, this, std::placeholders::_1);

	m_allocator = std::make_unique<FileblockAllocator>(*m_vdrive, fballoc_options{allocate, clear, free});
}

uint32_t FilesystemCore::create_inode(const fs_inode_options& options)
{
	if (m_sblock.s_tinode == 0) {
		throw std::runtime_error("Unable to create new inode entry. There are no free space in the table");
	}

	auto ctime = (uint32_t)std::time(nullptr);
	inode in{
		options.fs_attr,
		options.fs_uid,
		options.fs_gid,
		0,
		ctime,
		ctime,
		0,
		0
	};

	auto record = to_inode_record(in);
	record.fs_modified = true;

	auto inode_index = front_inode();
	pop_inode();

	m_icache.emplace(std::make_pair(inode_index, record));
	return inode_index;
}

uint32_t FilesystemCore::open_inode(uint32_t index)
{
	if (m_icache.count(index) != 0) {
		throw std::runtime_error("inode is already opened");
	}

	inode in = m_itable->read(index);
	FileAttr attr(in.i_attr);

	if (attr.deleted()) {
		throw std::runtime_error("inode is empty");
	}
	auto record = to_inode_record(in);
	m_icache.emplace(std::make_pair(index, record));
	return index;
}

const fs_inode_data FilesystemCore::get_inode(uint32_t index)
{
	auto it = get_cache_iterator(index);
	auto [index_, record] = *it;

	fs_inode_data data;
	memcpy(&data, &record.fs_inode, sizeof(fs_inode_data));
	return data;
}

void FilesystemCore::set_inode(uint32_t index, const fs_inode_options& options)
{
	auto it = get_cache_iterator(index);
	auto& record = it->second;
	auto& in = record.fs_inode;

	record.fs_modified = true;

	/* This code prevents modifying first two bits - directory bit & deleted bit
	 * directory bit remains untouched as it is
	 * deleted bit is always set to 1 which means that entry exists
	 */
	auto attr_mask = (in.i_attr & 0b1100000000000000) | 0b0100000000000000;
	auto attr_safe = options.fs_attr & 0b0011111111111111;

	in.i_attr = attr_mask | attr_safe;
	in.i_uid = options.fs_uid;
	in.i_gid = options.fs_gid;
}

void FilesystemCore::resize_inode(uint32_t index, uint32_t size, bool clear)
{
	auto it = get_cache_iterator(index);
	auto& record = it->second;
	auto& in = record.fs_inode;

	// Fix of precise ceil division without using double
	auto target_fsize = (size + m_sblock.s_bsize - 1) / m_sblock.s_bsize;
	auto current_fsize = in.i_fsize;

	in.i_size = size;
	record.fs_modified = true;

	m_allocator->resize(in, target_fsize, clear);
}

void FilesystemCore::close_inode(uint32_t index)
{
	auto it = get_cache_iterator(index);
	auto [index_, record] = *it;

	if (record.fs_modified) {
		auto mtime = (uint32_t)std::time(nullptr);
		auto in = record.fs_inode;
		in.i_mtime = mtime;
		m_itable->write(index, in);
	}
	m_icache.erase(it);
}

void FilesystemCore::free_inode(uint32_t index)
{
	if (m_icache.count(index) != 0) {
		throw std::runtime_error("Node is busy");
	}

	auto in = m_itable->read(index);
	auto attr = FileAttr(in.i_attr);

	in.i_attr = attr.deleted(true).value();
	in.i_size = 0;

	m_allocator->resize(in, 0);
	m_itable->write(index, in);
	push_inode(index);
}

void FilesystemCore::touch_inode(uint32_t index)
{
	auto it = get_cache_iterator(index);
	auto& record = it->second;
	record.fs_modified = true;
}

void FilesystemCore::link_inode(uint32_t index)
{
	auto it = get_cache_iterator(index);
	auto& record = it->second;
	auto& in = record.fs_inode;

	record.fs_modified = true;
	in.i_links++;
}

void FilesystemCore::unlink_inode(uint32_t index)
{
	auto it = get_cache_iterator(index);
	auto& record = it->second;
	auto& in = record.fs_inode;

	if (in.i_links == 0) throw std::exception("links is 0");
	record.fs_modified = true;
	in.i_links--;
}

bool FilesystemCore::is_open(uint32_t index)
{
	return m_icache.count(index) > 0;
}

uint32_t FilesystemCore::read(uint32_t index, char* data, uint32_t count)
{
	auto it = get_cache_iterator(index);
	auto& record = it->second;
	auto& in = record.fs_inode;

	// Out of file bounds
	if (record.fs_ptr >= in.i_size) return 0; 
	// Empty file
	if (in.i_size == 0) return 0;
	// No data to read
	if (count == 0) return 0;

	// Start block index in inode.i_blocks
	auto block_index = record.fs_ptr / m_sblock.s_bsize;

	// Real number of bytes to read
	uint32_t count_real = std::min(count, in.i_size - record.fs_ptr);
	// Not really need to be checked, but added to supress compiler warnings
	if (count_real == 0) return 0;

	// Current block index in inode.i_blocks
	uint32_t block_current;
	// Offset from start of block in bytes
	uint32_t byte_offset;
	// Total amount of bytes to be read from current block
	uint32_t byte_total;
	// Total remaining amount of bytes to read
	uint32_t byte_remain = count_real;

	std::vector<char> buffer(m_sblock.s_bsize, 0);
	do {
		if (byte_remain > 0) {
			block_current = m_allocator->get_cluster(in, block_index);
			byte_offset = record.fs_ptr - (block_index * m_sblock.s_bsize);
			byte_total = std::min(m_sblock.s_bsize - byte_offset, byte_remain);
		}

		m_vdrive->seek(block_current);
		m_vdrive->read(buffer.data());
		memcpy(data + (count - byte_remain), buffer.data() + byte_offset, byte_total);

		record.fs_ptr += byte_total;
		byte_remain -= byte_total;
		if (byte_remain == 0) break;

		block_index++;
	} while (1);
	return count_real;
}

void FilesystemCore::write(uint32_t index, const char* data, uint32_t count)
{
	auto it = get_cache_iterator(index);
	auto& record = it->second;
	auto& in = record.fs_inode;

	// No data to write
	if (count == 0) return;

	auto isize = std::max(in.i_size, (uint32_t)(record.fs_ptr + count));
	resize_inode(index, isize);

	// Start block index in inode.i_blocks
	auto block_index = record.fs_ptr / m_sblock.s_bsize;
	// Current block index in inode.i_blocks
	uint32_t block_current;
	// Offset from start of block in bytes
	uint32_t byte_offset;
	// Total amount of bytes to be written to current block
	uint32_t byte_total;
	// Total remaining amount of bytes to write
	uint32_t byte_remain = count;

	std::vector<char> buffer(m_sblock.s_bsize, 0);
	do {
		if (byte_remain > 0) {
			block_current = m_allocator->get_cluster(in, block_index);
			byte_offset = record.fs_ptr - (block_index * m_sblock.s_bsize);
			byte_total = std::min(m_sblock.s_bsize - byte_offset, byte_remain);
		}

		m_vdrive->seek(block_current);
		m_vdrive->read(buffer.data());
		memcpy(buffer.data() + byte_offset, data + (count - byte_remain), byte_total);
		m_vdrive->seek(block_current);
		m_vdrive->write(buffer.data());

		record.fs_ptr += byte_total;
		byte_remain -= byte_total;
		if (byte_remain == 0) break;

		block_index++;
	} while (1);
}

void FilesystemCore::seek(uint32_t index, uint32_t count, int origin)
{
	auto it = get_cache_iterator(index);
	auto& record = it->second;

	if (origin == FS_SEEK_SET) {
		record.fs_ptr = count;
		return;
	}
	if (origin == FS_SEEK_CUR) {
		record.fs_ptr += count;
		return;
	}
	record.fs_ptr = record.fs_inode.i_size + count;
}

uint32_t FilesystemCore::end(uint32_t index)
{
	auto it = get_cache_iterator(index);
	auto& record = it->second;
	return record.fs_inode.i_size;
}

uint32_t FilesystemCore::tell(uint32_t index)
{
	auto it = get_cache_iterator(index);
	auto& record = it->second;
	return record.fs_ptr;
}

const superblock& FilesystemCore::sblock() const
{
	return m_sblock;
}

const Drive& fs::internal::FilesystemCore::drive() const
{
	return *m_drive;
}

uint32_t FilesystemCore::front_inode()
{
	if (size_tinode() == 0) {
		rescan_itable();
		if (size_tinode() == 0) {
			throw std::runtime_error("Index out of bounds. There are no free inodes in the table");
		}
	}
	auto nindex = size_tinode() - 1;
	return m_sblock.s_inode[nindex];
}

void FilesystemCore::pop_inode()
{
	if (size_tinode() == 0) {
		rescan_itable();
		if (size_tinode() == 0) {
			throw std::runtime_error("Index out of bounds. There are no free inodes in the table");
		}
	}
	m_sblock.s_tinode--;
	m_sblock.s_ttinode--;
	write_superblock(m_drive, m_sblock);
}

void FilesystemCore::push_inode(uint32_t index)
{
	if (size_tinode() == INODE_TABLE_SIZE) {
		return;
	}
	auto nindex = size_tinode();
	m_sblock.s_inode[nindex] = index;
	m_sblock.s_tinode++;
	write_superblock(m_drive, m_sblock);
}

uint32_t FilesystemCore::size_tinode()
{
	return m_sblock.s_ttinode;
}

uint32_t FilesystemCore::allocate_cluster()
{
	auto address = m_ctable->front();
	m_ctable->pop();
	m_sblock.s_tfree--;
	write_superblock(m_drive, m_sblock);
	return address;
}

void FilesystemCore::free_cluster(uint32_t address)
{
	m_ctable->push(address);
	m_sblock.s_tfree++;
	write_superblock(m_drive, m_sblock);
}

void FilesystemCore::clear_cluster(uint32_t address)
{
	std::vector<char> buf(m_sblock.s_bsize, 0);
	m_vdrive->seek(address);
	m_vdrive->write(buf.data());
}

void FilesystemCore::rescan_itable()
{
	m_sblock.s_tinode = 0;
	auto itotal = m_itable->total_nodes();

	while (itotal > 0) {
		auto inode = m_itable->read(itotal - 1);
		auto attr = FileAttr(inode.i_attr);

		if (attr.deleted()) {
			if (m_sblock.s_tinode < INODE_TABLE_SIZE) {
				m_sblock.s_inode[m_sblock.s_tinode] = itotal - 1;
			}
			m_sblock.s_tinode++;
		}

		itotal--;
	}
	m_sblock.s_ttinode = std::min(m_sblock.s_tinode, (uint32_t)INODE_TABLE_SIZE);
	write_superblock(m_drive, m_sblock);
}
