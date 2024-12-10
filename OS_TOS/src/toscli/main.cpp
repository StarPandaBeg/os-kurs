#include <iostream>
#include <memory>

#include "CLI11.hpp"
#include "drive.h"
#include "filesystem.h"
#include "shell.h"

#include "command_drive.h"
#include "command_fs.h"

#define TOSTOOL_VERSION "0.0.1"

using namespace CLI;
using namespace std;

void drive_create(const drive_create_options& opts);
void drive_info(const drive_info_options& opts);
void fs_format(const fs_format_options& opts);
void fs_shell(const fs_shell_options& opts);

int main(int argc, char** argv) {
	App app{ "Universal tool to install and manage tOS"};
	app.set_version_flag("-v", TOSTOOL_VERSION);
	argv = app.ensure_utf8(argv);

	auto drive = app.add_subcommand("drive", "Manage filesystem drives");
	auto fs = app.add_subcommand("fs", "Manage drive filesystem");
	command_drive_setup(drive);
	command_fs_setup(fs);

	command_drive_on_create(drive_create);
	command_drive_on_info(drive_info);
	command_fs_on_format(fs_format);
	command_fs_on_shell(fs_shell);

	CLI11_PARSE(app, argc, argv);
	return 0;
}

void drive_create(const drive_create_options& opts) {
	try {
		cout << "Creating new disk at " << opts.path << endl;
		cout << "Sector size: " << opts.sector << endl;
		cout << "Disk size (in sectors): " << opts.size << endl;
		cout << "Disk size (in bytes): " << opts.size * opts.sector << endl << endl;

		fs::Drive::create(opts.path, {opts.sector, opts.size});
		cout << "Disk created successfully";
	}
	catch (exception& e) {
		std::cerr << "Unable to create disk: " << e.what();
	}
}

void drive_info(const drive_info_options& opts) {
	try {
		auto drive = fs::Drive(opts.path);
		auto dopts = drive.options();

		cout << "Disk " << opts.path << endl;
		cout << "Sector size: " << dopts.v_sec << endl;
		cout << "Disk size (in sectors): " << dopts.v_size << endl;
		cout << "Disk size (in bytes): " << dopts.v_size * dopts.v_sec;
	}
	catch (exception& e) {
		std::cerr << "Unable to open disk: " << e.what();
	}
}

void fs_format(const fs_format_options& opts)
{
	try {
		auto drive = std::make_shared<fs::Drive>(opts.path);
		auto dopts = drive->options();

		cout << "Disk " << opts.path << endl;
		cout << "Sector size: " << dopts.v_sec << endl;
		cout << "Disk size (in sectors): " << dopts.v_size << endl;
		cout << "Disk size (in bytes): " << dopts.v_size * dopts.v_sec << endl;

		cout << "FS cluster size: " << opts.block << endl;
		cout << "FS inode table size (in clusters): " << opts.inode_size << endl;

		if (opts.clear) {
			std::vector<char> buffer(dopts.v_sec, 0);
			for (uint32_t i = 0; i < dopts.v_size; i++) {
				drive->write(buffer.data());
			}
			drive->seek(0);
		}
		fs::Filesystem::format(drive, {opts.block, opts.inode_size});
		cout << "Format successfull";
	}
	catch (exception& e) {
		std::cerr << "Unable to format filesystem: " << e.what();
	}
}

void fs_shell(const fs_shell_options& opts) {
	try {
		auto drive = std::make_shared<fs::Drive>(opts.path);
		auto fs = fs::Filesystem(drive);

		if (!opts.command.empty()) {
			execute(fs, opts.command);
			return;
		}
		shell(fs);
	}
	catch (exception& e) {
		std::cerr << "Unable to open shell: " << e.what();
	}
}