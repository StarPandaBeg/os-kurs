#pragma once

#include "CLI11.hpp"

struct drive_create_options {
	uint16_t sector;
	uint32_t size = 0;
	std::string path;
	bool overwrite = false;
};

struct drive_info_options {
	std::string path;
};

void command_drive_setup(CLI::App* app);
void command_drive_on_create(std::function<void(const drive_create_options& options)> callback);
void command_drive_on_info(std::function<void(const drive_info_options& path)> callback);