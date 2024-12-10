#pragma once

#include <cstdint>
#include <fstream>

#define DRIVE_FILE_VERSION 1
#define DRIVE_HEADER_SIZE sizeof(drive_options)
#define DRIVE_HEADER_REAL_SIZE (DRIVE_HEADER_SIZE + 2)

namespace fs {
#pragma pack(push, 1)
	typedef struct drive_options {
		uint16_t v_sec = 512;      // Sector size (256, 512, 1024, 2048)
		uint32_t v_size = 0;       // Drive size (in sectors)
	} drive_options;
#pragma pack(pop)

	class Drive {
	public:
		static Drive create(std::string path, const drive_options& options);

		Drive(std::string path);
		Drive(const Drive& other);
		~Drive();

		const drive_options& options() const;

		void seek(std::streampos pos);
		std::streampos tell();

		bool fail() const;
		void clear();

		void read(char* str);
		void write(char* str);
	private:
		std::fstream m_file;
		std::string m_path;
		drive_options m_options;
		bool m_fail;

		void read_header();
		void set_buffering(std::streamsize size);
	};
}