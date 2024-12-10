#pragma once

#include <map>
#include <memory>
#include "filesystem.h"

#define PERM_READ    0
#define PERM_WRITE   1
#define PERM_EXECUTE 2

namespace tos {
	class TOS {
	public:
		typedef struct filemode {
			bool exist;
			bool read;
			bool write;
			bool extend;
			bool append;
		} filemode;

		TOS(std::unique_ptr<fs::Filesystem> fs);

		bool can(std::string path, uint16_t uid, std::set<uint16_t>& gids, uint8_t what);
		fs::Filesystem& fs();

		filemode to_mode(std::string mode);
		uint32_t open(std::string path, std::string mode);
		void close(uint32_t handle);
		uint32_t read(uint32_t handle, char* data, uint32_t count);
		void write(uint32_t handle, const char* data, uint32_t count);
		void seek(uint32_t handle, uint32_t count, int origin = FS_SEEK_SET);
		uint32_t tell(uint32_t handle);
		uint32_t end(uint32_t handle);
	private:
		std::unique_ptr<fs::Filesystem> m_fs;
		std::map<uint32_t, std::string> m_files;
	};
}