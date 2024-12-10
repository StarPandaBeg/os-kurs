#pragma once

#include "blocks.h"

namespace fs {
	class FileAttr {
	public:
		FileAttr(uint16_t raw);
		FileAttr();

		bool is_dir() const;
		bool is_file() const;
		bool deleted() const;

		bool suid() const;
		bool sgid() const;
		bool sticky() const;

		/// index: 0 - read, 1 - write, 2 - execute
		bool owner(uint8_t index) const;
		/// index: 0 - read, 1 - write, 2 - execute
		bool group(uint8_t index) const;
		/// index: 0 - read, 1 - write, 2 - execute
		bool world(uint8_t index) const;

		/// who:  0 - owner, 1 - group, 2 - world
		/// what: 0 - read, 1 - write, 2 - execute
		bool can(uint8_t who, uint8_t what) const;

		FileAttr& is_dir(bool value);
		FileAttr& is_file(bool value);
		FileAttr& deleted(bool value);

		FileAttr& suid(bool value);
		FileAttr& sgid(bool value);
		FileAttr& sticky(bool value);

		/// index: 0 - read, 1 - write, 2 - execute
		FileAttr& owner(uint8_t index, bool value);
		/// index: 0 - read, 1 - write, 2 - execute
		FileAttr& group(uint8_t index, bool value);
		/// index: 0 - read, 1 - write, 2 - execute
		FileAttr& world(uint8_t index, bool value);

		/// who:  0 - owner, 1 - group, 2 - world
		/// what: 0 - read, 1 - write, 2 - execute
		FileAttr& can(uint8_t who, uint8_t what, bool value);

		const uint16_t value() const;
	private:
		uint16_t m_raw;

		bool is_set(uint8_t index) const;
		void set(uint8_t index, bool value);
	};
}