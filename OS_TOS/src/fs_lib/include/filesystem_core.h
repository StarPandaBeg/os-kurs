#pragma once

#include <map>
#include <memory>
#include "blocks.h"
#include "cluster.h"
#include "drive.h"
#include "fileblock.h"
#include "inode.h"
#include "virtual_drive.h"

#define FS_MAGIC 0x534f
#define FS_ATTR_DEFAULT 0b0100000110100000 // -rw-r-----

#define FS_SEEK_SET 0
#define FS_SEEK_CUR 1
#define FS_SEEK_END 2

namespace fs {
#pragma pack(push, 1)
	typedef struct fs_format_options {
		uint16_t fs_bsize = 512;            // Disk block size (512, 1024, 2048, 4096)
		uint32_t fs_isize;                  // inode block count
	} fs_format_options;

	typedef struct fs_inode_options {
		uint16_t fs_attr;
		uint16_t fs_uid;
		uint16_t fs_gid;
	} fs_inode_options;

	typedef struct fs_inode_data : fs_inode_options {
		uint16_t fs_links;
		uint32_t fs_ctime;
		uint32_t fs_mtime;
		uint32_t fs_size;
		uint32_t fs_fsize;
	} fs_inode_data;

	typedef struct fs_inode_record {
		inode fs_inode;
		bool fs_modified;
		uint32_t fs_ptr;
	} fs_inode_record;
#pragma pack(pop)

	namespace internal {
		using ICache = std::map<uint32_t, fs_inode_record>;

		class FilesystemCore {
		public:
			static void format(std::shared_ptr<Drive> drive, const fs_format_options& options);

			FilesystemCore(std::shared_ptr<Drive> drive);

			uint32_t create_inode(const fs_inode_options& options);
			uint32_t open_inode(uint32_t index);
			const fs_inode_data get_inode(uint32_t index);
			void set_inode(uint32_t index, const fs_inode_options& options);
			void resize_inode(uint32_t index, uint32_t size, bool clear = false);
			void close_inode(uint32_t index);
			void free_inode(uint32_t index);
			void touch_inode(uint32_t index);
			void link_inode(uint32_t index);
			void unlink_inode(uint32_t index);

			bool is_open(uint32_t index);

			uint32_t read(uint32_t index, char* data, uint32_t count);
			void write(uint32_t index, const char* data, uint32_t count);
			void seek(uint32_t index, uint32_t count, int origin = FS_SEEK_SET);
			uint32_t end(uint32_t index);
			uint32_t tell(uint32_t index);

			const superblock& sblock() const;
			const Drive& drive() const;
		private:
			std::shared_ptr<Drive> m_drive;
			superblock m_sblock;
			std::unique_ptr<internal::VirtualDrive> m_vdrive;
			std::unique_ptr<internal::INodeTable> m_itable;
			std::unique_ptr<internal::ClusterTable> m_ctable;
			std::unique_ptr<internal::FileblockAllocator> m_allocator;
			ICache m_icache;

			static void read_superblock(std::shared_ptr<Drive> drive, superblock& sb);
			static void write_superblock(std::shared_ptr<Drive> drive, const superblock& sb);

			ICache::iterator get_cache_iterator(uint32_t index);

			uint32_t front_inode();
			void pop_inode();
			void push_inode(uint32_t index);
			uint32_t size_tinode();

			uint32_t allocate_cluster();
			void free_cluster(uint32_t address);
			void clear_cluster(uint32_t address);

			void rescan_itable();
		};
	}
}