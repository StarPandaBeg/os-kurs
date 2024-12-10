#pragma once

#include <string>
#include <vector>

#define PATH_SEPARATOR '/'

namespace fs {
	class Path {
	public:
		Path(const std::string& path);
		Path(const char* path);

		void set(const std::string& path);
		std::string get() const;
		std::string filename() const;
		std::string filename_level(size_t level) const;
		std::string get_level(size_t level) const;
		void move_up();
		void add_component(const std::string& component);

		const std::vector<std::string>& components() const;
		const std::size_t level() const;
	private:
		std::string m_full_path;
		std::vector<std::string> m_components;

		void parse_path(const std::string& path);
		void update_full_path();
		bool is_absolute_path(const std::string& path) const;
	};
}