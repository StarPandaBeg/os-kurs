#pragma once

#include <cstdint>
#include "virtual_drive.h"

namespace fs {
	namespace internal {
		typedef struct vpartition_options {
			uint32_t v_bsize;
			uint32_t v_offset = 0;
			uint32_t v_size = 0;
		} vpartition_options;

		class VPartition {
		public:
			VPartition(VirtualDrive& vdrive, const vpartition_options& options);
		protected:
			VirtualDrive& m_vdrive;
			const vpartition_options m_options;

			uint32_t cluster_index(uint32_t index);
			uint32_t relative_index(uint32_t index, uint32_t cindex);

			virtual uint32_t block_size() = 0;
		};
	}
}