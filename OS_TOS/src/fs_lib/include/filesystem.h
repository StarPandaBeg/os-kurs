#pragma once

#include "catalog.h"
#include "filesystem_core.h"
#include "path.h"
#include "tree.h"

namespace fs {
	namespace internal {
		using FileTree = Tree<uint32_t>;
	}

	class Filesystem {
	public:
		static void format(std::shared_ptr<Drive> drive, const fs_format_options& options);

		Filesystem(std::shared_ptr<Drive> drive);

		uint32_t open(Path path);
		void close(uint32_t index);
		uint32_t read(uint32_t index, char* data, uint32_t count);
		void write(uint32_t index, const char* data, uint32_t count);
		void seek(uint32_t index, uint32_t count, int origin = FS_SEEK_SET);
		uint32_t end(uint32_t index);
		uint32_t tell(uint32_t index);
		void clear(uint32_t index);
		void touch(uint32_t index);

		bool is_open(uint32_t index);

		const fs_inode_data filedata(uint32_t index);
		void attr(uint32_t index, uint16_t attr);
		void user(uint32_t index, uint16_t uid);
		void group(uint32_t index, uint16_t gid);

		void create_catalog(Path path, uint16_t owner = 0, uint16_t group = 0);
		const std::vector<std::string> list_catalog(Path path);
		void remove(Path path);
		bool is_file(Path path);
		bool is_catalog(Path path);
		bool exists(Path path);
		void touch(Path path);
		void link(Path path, Path link);
		void unlink(Path path);
		const fs_inode_data filedata(Path path);

		internal::FilesystemCore& core();
	private:
		internal::FilesystemCore m_fcore;
		internal::FileTree m_tree;

		internal::CatalogHandle create_catalog(const fs_inode_options& options);
		internal::CatalogHandle open_catalog(uint32_t index);
		void close_catalog(internal::CatalogHandle& index);

		bool is_file(uint32_t index);
		bool is_catalog(uint32_t index);
		bool exists(uint32_t index);
		void remove(uint32_t index, uint32_t pindex);
		void link(uint32_t index, Path link);
		void unlink(uint32_t index, Path link);

		uint32_t find_index(Path path);
		int find_cached_level(Path path, uint32_t& address);
		void clear_tree();
	};
}