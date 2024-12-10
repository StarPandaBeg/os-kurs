#pragma once

#include "drive.h"

namespace fs {
	namespace internal {
		typedef struct vdrive_options {
			uint16_t v_bsize;
			uint32_t v_offset;
			uint32_t v_fsize;
		} vdrive_options;

		class VirtualDrive {
		public:
			VirtualDrive(std::shared_ptr<Drive> drive, const vdrive_options& options);

			void seek(std::streampos pos);
			std::streampos tell();

			bool fail();
			void clear();

			void read(char* str);
			void write(char* str);

			const vdrive_options& options() const;
		private:
			std::shared_ptr<Drive> m_drive;
			const vdrive_options m_options;

			uint16_t bcoef();
		};
	}
}