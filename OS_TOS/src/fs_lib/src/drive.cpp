#include "drive.h"

#include <filesystem>

using namespace fs;

Drive::Drive(std::string path): m_path(path)
{
	if (!std::filesystem::exists(path)) {
		throw std::runtime_error("Drive file is not exist");
	}
	m_file = std::fstream(path, std::ios::in | std::ios::out | std::ios::binary);
	
	read_header();
	set_buffering(m_options.v_sec);
}

Drive::Drive(const Drive& other) : Drive(other.m_path) {}

Drive::~Drive()
{
	if (m_file.is_open()) {
		m_file.close();
	}
}

Drive Drive::create(std::string path, const drive_options& options)
{
	auto file = std::fstream(path, std::ios::out | std::ios::binary);
	if (file.fail()) {
		throw std::runtime_error("Unable to create volume file");
	}

	// Do not try to refactor with "file << (uint16_t)DRIVE_FILE_VERSION"
	// Fallback due to operator<< overloaded for uint16_t to write string instead of bytes
	auto file_version = (uint16_t)DRIVE_FILE_VERSION;
	file.write(reinterpret_cast<char*>(&file_version), sizeof(file_version));
	file.write(reinterpret_cast<const char*>(&options), DRIVE_HEADER_SIZE);

	std::vector<char> buffer(options.v_sec, 0);
	for (uint32_t i = 0; i < options.v_size; i++)
	{
		file.write(buffer.data(), options.v_sec);
	}	
	
	file.close();
	return Drive(std::move(path));
}

void Drive::seek(std::streampos pos)
{
	std::streampos realpos = DRIVE_HEADER_REAL_SIZE + pos * m_options.v_sec;
	m_file.seekg(realpos);
}

std::streampos Drive::tell()
{
	std::streampos realpos = m_file.tellg() - std::streampos{ DRIVE_HEADER_REAL_SIZE };
	return realpos / m_options.v_sec;
}

bool fs::Drive::fail() const
{
	return m_file.fail() || m_fail;
}

void Drive::clear()
{
	m_file.clear();
	m_fail = false;
}

void Drive::read(char* str)
{
	if (tell() >= m_options.v_size) {
		m_fail = true;
		return;
	}
	m_file.read(str, m_options.v_sec);
}

void Drive::write(char* str)
{
	if (tell() >= m_options.v_size) {
		m_fail = true;
		return;
	}
	m_file.write(str, m_options.v_sec);
}

void Drive::read_header()
{
	char header_buf[DRIVE_HEADER_SIZE];
	m_file.seekg(2); // Skip file version 
	m_file.read(header_buf, DRIVE_HEADER_SIZE);
	m_options = *reinterpret_cast<drive_options*>(header_buf);
}

void Drive::set_buffering(std::streamsize size)
{
	auto pos = m_file.tellg();
	m_file.rdbuf()->pubsetbuf(nullptr, size);
	m_file.seekg(pos);
}

const drive_options& Drive::options() const
{
	return m_options;
}
