#include "virtual_drive.h"

using namespace fs;
using namespace fs::internal;

VirtualDrive::VirtualDrive(std::shared_ptr<Drive> drive, const vdrive_options& options) : m_drive(drive), m_options(options)
{
	auto dopts = drive->options();
	if (options.v_bsize % dopts.v_sec != 0 || dopts.v_sec > options.v_bsize) {
		throw std::runtime_error("Invalid disk block size. It must be a multiple of the disk sector size.");
	}
}

void VirtualDrive::seek(std::streampos pos)
{
	if (pos >= m_options.v_fsize) {
		throw std::runtime_error("Index out of bounds");
	}
	std::streampos spos = pos * bcoef() + m_options.v_offset;
	m_drive->seek(spos);
}

std::streampos VirtualDrive::tell()
{
	std::streampos spos = m_drive->tell() - std::streampos{ m_options.v_offset };
	if (spos % bcoef() != 0) {
		throw std::runtime_error("Invalid disk block index");
	}
	return spos / bcoef();
}

bool VirtualDrive::fail()
{
	return m_drive->fail();
}

void VirtualDrive::clear()
{
	return m_drive->clear();
}

void VirtualDrive::read(char* str)
{
	std::streampos spos = m_drive->tell() - std::streampos{ m_options.v_offset };
	if (spos % bcoef() != 0) {
		throw std::runtime_error("Invalid disk block index");
	}
	auto dopts = m_drive->options();

	for (uint16_t i = 0; i < m_options.v_bsize; i += dopts.v_sec) {
		m_drive->read(str + i);
	}
}

void VirtualDrive::write(char* str)
{
	std::streampos spos = m_drive->tell() - std::streampos{ m_options.v_offset };
	if (spos % bcoef() != 0) {
		throw std::runtime_error("Invalid disk block index");
	}
	auto dopts = m_drive->options();

	for (uint16_t i = 0; i < m_options.v_bsize; i += dopts.v_sec) {
		m_drive->write(str + i);
	}
}

const vdrive_options& VirtualDrive::options() const
{
	return m_options;
}

uint16_t VirtualDrive::bcoef()
{
	return m_options.v_bsize / m_drive->options().v_sec;
}
