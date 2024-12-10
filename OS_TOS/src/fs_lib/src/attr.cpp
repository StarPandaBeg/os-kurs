#include "attr.h"
#include "bit_util.h"

using namespace fs;
using namespace fs::internal;

#define BIT_DIRECTORY  15
#define BIT_DELETED    14
#define BIT_SUID       11
#define BIT_SGID       10
#define BIT_STICKY     9
#define BIT_OWNER_R    8
#define BIT_OWNER_W    7
#define BIT_OWNER_X    6
#define BIT_GROUP_R    5
#define BIT_GROUP_W    4
#define BIT_GROUP_X    3
#define BIT_WORLD_R    2
#define BIT_WORLD_W    1
#define BIT_WORLD_X    0

FileAttr::FileAttr(const uint16_t raw) : m_raw(raw) {}

FileAttr::FileAttr() : FileAttr(0)
{
}

bool FileAttr::is_dir() const
{
	return is_set(BIT_DIRECTORY);
}

bool FileAttr::is_file() const
{
	return !is_dir();
}

bool FileAttr::deleted() const
{
	return !is_set(BIT_DELETED);
}

bool FileAttr::suid() const
{
	return is_set(BIT_SUID);
}

bool FileAttr::sgid() const
{
	return is_set(BIT_SGID);
}

bool FileAttr::sticky() const
{
	return is_set(BIT_STICKY);
}

bool FileAttr::owner(uint8_t index) const
{
	return is_set(BIT_OWNER_R - index);
}

bool FileAttr::group(uint8_t index) const
{
	return is_set(BIT_GROUP_R - index);
}

bool FileAttr::world(uint8_t index) const
{
	return is_set(BIT_WORLD_R - index);
}

bool FileAttr::can(uint8_t who, uint8_t what) const
{
	uint8_t offset = 2 + 3 * (2 - who);
	return is_set(offset - what);
}

FileAttr& FileAttr::is_dir(bool value)
{
	set(BIT_DIRECTORY, value);
	return *this;
}

FileAttr& FileAttr::is_file(bool value)
{
	return is_dir(!value);
}

FileAttr& FileAttr::deleted(bool value)
{
	set(BIT_DELETED, !value);
	return *this;
}

FileAttr& FileAttr::suid(bool value)
{
	set(BIT_SUID, value);
	return *this;
}

FileAttr& FileAttr::sgid(bool value)
{
	set(BIT_SGID, value);
	return *this;
}

FileAttr& FileAttr::sticky(bool value)
{
	set(BIT_STICKY, value);
	return *this;
}

FileAttr& FileAttr::owner(uint8_t index, bool value)
{
	set(BIT_OWNER_R - index, value);
	return *this;
}

FileAttr& FileAttr::group(uint8_t index, bool value)
{
	set(BIT_GROUP_R - index, value);
	return *this;
}

FileAttr& FileAttr::world(uint8_t index, bool value)
{
	set(BIT_GROUP_R - index, value);
	return *this;
}

FileAttr& FileAttr::can(uint8_t who, uint8_t what, bool value)
{
	uint8_t offset = 2 + 3 * (2 - who);
	set(offset - what, value);
	return *this;
}

const uint16_t FileAttr::value() const
{
	return m_raw;
}

bool FileAttr::is_set(uint8_t index) const
{
	return binary_is_set(m_raw, index);
}

void FileAttr::set(uint8_t index, bool value)
{
	binary_toggle(m_raw, index, value);	
}
