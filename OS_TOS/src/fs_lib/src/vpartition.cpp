#include "vpartition.h"

using namespace fs::internal;

VPartition::VPartition(VirtualDrive& vdrive, const vpartition_options& options) : m_vdrive(vdrive), m_options(options)
{
}

uint32_t VPartition::cluster_index(uint32_t index)
{
	auto block_per_cluster = m_options.v_bsize / block_size();
	return index / block_per_cluster + m_options.v_offset;
}

uint32_t VPartition::relative_index(uint32_t index, uint32_t cindex)
{
	auto block_per_cluster = m_options.v_bsize / block_size();
	auto rindex = index - block_per_cluster * (cindex - m_options.v_offset);
	return rindex * block_size();
}
