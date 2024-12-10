#pragma once

#include "CLI11.hpp"

struct fs_format_options {
	uint16_t block;
	uint32_t inode_size;
	std::string path;
	bool clear = false;
};
struct fs_shell_options {
	std::string path;
	std::string command;
};

void command_fs_setup(CLI::App* app);
void command_fs_on_format(std::function<void(const fs_format_options& options)> callback);
void command_fs_on_shell(std::function<void(const fs_shell_options& options)> callback);
