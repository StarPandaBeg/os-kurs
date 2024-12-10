#include "cluster.h"

#include <vector>

using namespace fs::internal;

ClusterTable::ClusterTable(VirtualDrive& vdrive, const vpartition_options& options, uint32_t tfree) : VPartition(vdrive, options), m_tfree(tfree)
{
	sync();
}

void ClusterTable::push(uint32_t address)
{
	if (m_tfree == m_vdrive.options().v_fsize) {
		throw std::runtime_error("Index out of bounds. There are no free space in the table");
	}

	auto cindex = cluster_index(m_tfree);
	auto bindex = relative_index(m_tfree, cindex);

	std::vector<char> buffer(m_options.v_bsize, 0);
	m_vdrive.seek(cindex);
	m_vdrive.read(buffer.data());

	memcpy(buffer.data() + bindex, &address, sizeof(uint32_t));

	m_vdrive.seek(cindex);
	m_vdrive.write(buffer.data());

	m_tfree++;
	m_fvalue = address;
}

uint32_t ClusterTable::front()
{
	if (m_tfree == 0) {
		throw std::runtime_error("Index out of bounds. There are no free nodes in the table");
	}
	return m_fvalue;
}

void ClusterTable::pop()
{
	if (m_tfree == 0) {
		throw std::runtime_error("Index out of bounds. There are no free nodes in the table");
	}
	m_tfree--;
	sync();
}

uint32_t ClusterTable::count()
{
	return m_tfree;
}

uint32_t ClusterTable::block_size()
{
	return CLUSTER_RECORD_SIZE;
}

void ClusterTable::sync()
{
	if (m_tfree == 0) {
		return;
	}

	auto cindex = cluster_index(m_tfree - 1);
	auto bindex = relative_index(m_tfree - 1, cindex);

	std::vector<char> buffer(m_options.v_bsize, 0);
	m_vdrive.seek(cindex);
	m_vdrive.read(buffer.data());

	memcpy(&m_fvalue, buffer.data() + bindex, sizeof(m_fvalue));
}
