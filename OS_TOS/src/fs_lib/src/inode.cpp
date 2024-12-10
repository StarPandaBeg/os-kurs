#include "inode.h"

#include <vector>

using namespace fs::internal;

INodeTable::INodeTable(VirtualDrive& vdrive, const vpartition_options& options) : VPartition(vdrive, options)
{
}

inode INodeTable::read(uint32_t index)
{
	if (index >= total_nodes()) {
		throw std::runtime_error("Index out of bounds");
	}

	auto cindex = cluster_index(index);
	auto rindex = relative_index(index, cindex);

	inode inode;

	std::vector<char> buffer(m_options.v_bsize, 0);
	m_vdrive.seek(cindex);
	m_vdrive.read(buffer.data());

	memcpy(&inode, buffer.data() + rindex, INODE_SIZE);
	return inode;
}

void INodeTable::write(uint32_t index, inode inode)
{
	if (index >= total_nodes()) {
		throw std::runtime_error("Index out of bounds");
	}

	auto cindex = cluster_index(index);
	auto rindex = relative_index(index, cindex);

	std::vector<char> buffer(m_options.v_bsize, 0);
	m_vdrive.seek(cindex);
	m_vdrive.read(buffer.data());

	memcpy(buffer.data() + rindex, &inode, INODE_SIZE);
	
	m_vdrive.seek(cindex);
	m_vdrive.write(buffer.data());
}

uint32_t INodeTable::total_nodes()
{
	auto inode_per_cluster = m_options.v_bsize / INODE_SIZE;
	return m_options.v_size * inode_per_cluster;
}

uint32_t INodeTable::block_size()
{
	return INODE_SIZE;
}