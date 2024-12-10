#pragma once

#include <map>
#include <string>
#include <set>
#include <vector>
#include "filesystem_core.h"

namespace fs {
	namespace internal {
#pragma pack(push, 1)
		typedef struct catalog_entry {
			uint32_t c_inode;                        // inode index
			char c_name[28];                         // Filename
		} catalog_entry;

		typedef struct catalog_record {
			catalog_entry c_entry;
			bool c_modified;
		} catalog_record;
#pragma pack(pop)

		class CatalogHandle {
		public:
			CatalogHandle(FilesystemCore& fcore, const uint32_t index);

			void add(uint32_t index, std::string filename);
			bool exists(std::string filename);
			int remove(uint32_t index);
			int remove(std::string filename);
			uint32_t get(std::string filename);
			std::string get(uint32_t index);

			const std::vector<catalog_entry> list();

			uint32_t index() const;
			void close();
		private:
			const uint32_t m_index;
			FilesystemCore& m_fcore;
			// std::multimap<uint32_t, catalog_record> m_cache;
			// std::set<std::string> m_ncache;

			// std::map<uint32_t, std::string> m_cache2;
			std::map<std::string, catalog_record> m_cache3;
			std::multimap<uint32_t, std::string> m_cache2;

			void scan();
			uint32_t count(bool cached = true);
		};
	}
}