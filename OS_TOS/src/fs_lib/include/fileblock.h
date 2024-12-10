#pragma once

#include <functional>
#include "blocks.h"
#include "vpartition.h"

namespace fs {
	namespace internal {
		typedef struct fballoc_options {
			std::function<uint32_t()> allocate;
			std::function<void(uint32_t)> clear;
			std::function<void(uint32_t)> free;
		} fballoc_options;

		class FileblockAllocator {
		public:
			FileblockAllocator(VirtualDrive& vdrive, const fballoc_options& opts);

			void resize(inode& inode, uint32_t size, bool clear = false);
			void set_cluster(inode& inode, uint32_t index, uint32_t address);
			uint32_t get_cluster(inode& inode, uint32_t index);
		private:
			VirtualDrive& m_vdrive;
			uint16_t m_bsize;
			uint16_t m_acount;
			std::function<uint32_t()> m_allocate;
			std::function<void(uint32_t)> m_clear;
			std::function<void(uint32_t)> m_free;

			void extend(inode& inode, uint32_t count, bool clear = false);
			void shrink(inode& inode, uint32_t count);

			uint8_t get_indirection_level(uint32_t index);
			void set_indirection_cluster(inode& inode, uint32_t rindex, uint8_t level, uint32_t address);
			uint32_t set_indirection_cluster_address(inode& inode, uint32_t rindex, uint8_t level, uint32_t address, uint32_t baddress);
			uint32_t get_indirection_cluster(inode& inode, uint32_t rindex, uint8_t level);
			uint32_t get_indirection_cluster(inode& inode, uint32_t rindex, uint8_t level, uint32_t baddress);

			void free_cluster(inode& inode, uint32_t index);
			void free_indirection_cluster(inode& inode, uint32_t rindex, uint8_t level);
			void free_indirection_cluster(inode& inode, uint32_t rindex, uint8_t level, uint32_t baddress);
		};
	}
}