#pragma once

#include "blocks.h"
#include "vpartition.h"

namespace fs {
	namespace internal {
		class ClusterTable : public VPartition {
		public:
			ClusterTable(VirtualDrive& vdrive, const vpartition_options& options, uint32_t tfree);

			void push(uint32_t address);
			uint32_t front();
			void pop();

			uint32_t count();
		protected:
			uint32_t block_size() override;
		private:
			uint32_t m_tfree = 0;
			uint32_t m_fvalue = 0;

			void sync();
		};
	}
}