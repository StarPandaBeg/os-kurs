#include "command_fs.h"
#include "blocks.h"
#include "drive.h"

using namespace CLI;
using namespace fs;

struct fs_callbacks {
	std::function<void(const fs_format_options& options)> format;
	std::function<void(const fs_shell_options& options)> shell;
};

fs_format_options format_options;
fs_shell_options shell_options;
fs_callbacks callbacks;

void fs_format_setup(App* parent);
void fs_format_set_inodes(const int& value);
void fs_shell_setup(App* parent);

void command_fs_setup(App* app)
{
	fs_format_setup(app);
	fs_shell_setup(app);
}

void command_fs_on_format(std::function<void(const fs_format_options& options)> callback)
{
	callbacks.format = callback;
}

void command_fs_on_shell(std::function<void(const fs_shell_options& options)> callback)
{
	callbacks.shell = callback;
}

void fs_format_setup(App* parent)
{
	auto app = parent->add_subcommand("format", "Format drive");

	app->add_flag("-f,--full", format_options.clear, "Fill disk with 00 before formatting");
	app->add_option("filename", format_options.path, "Path to disk file")
		->required()
		->check(ExistingFile);
	app->add_option("-s,--sector", format_options.block, "Virtual sector size (512,1024,...,4096)")
		->default_val(512)
		->check(Range(512, 4096))
		->check([](const std::string& value) {
			Drive d(format_options.path);
			auto dopts = d.options();
			int int_value = std::stoi(value);
			if (int_value % dopts.v_sec != 0) {
				return "Value must be a mupliple of disk sector size (" + std::to_string(dopts.v_sec) + ")";
			}
			return std::string();
		});
	app->add_option_function<int>("-i,--inodes", fs_format_set_inodes, "Number of inodes to be created in the file system. At least the specified number will be created, but there could be more.")
		->check(PositiveNumber);
	app->add_option("--isectors", format_options.inode_size, "Number of virtual sectors to allocate for inode table")
		->excludes("-i")
		->check(PositiveNumber);

	app->callback([]() {
		if (format_options.inode_size == 0) {
			throw ValidationError("inode table size cannot be zero");
		}
		callbacks.format(format_options);
	});
}

void fs_format_set_inodes(const int& value)
{
	format_options.inode_size = (uint32_t)std::ceil((float)value / format_options.block);
}

void fs_shell_setup(App* parent)
{
	auto app = parent->add_subcommand("shell", "Interactive shell");
	app->add_option("filename", shell_options.path, "Path to disk file")
		->required()
		->check(ExistingFile);
	app->add_option("-c,--command", shell_options.command, "Execute one command and exit");
	app->callback([]() {
		callbacks.shell(shell_options);
	});
}
