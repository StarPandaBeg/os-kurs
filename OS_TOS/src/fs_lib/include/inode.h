#pragma once

#include "blocks.h"
#include "vpartition.h"

namespace fs {
	namespace internal {
		class INodeTable : public VPartition {
		public:
			INodeTable(VirtualDrive& vdrive, const vpartition_options& options);

			inode read(uint32_t index);
			void write(uint32_t index, inode inode);

			uint32_t total_nodes();
		protected:
			uint32_t block_size() override;
		};
	}
}