#include "command_drive.h"

#include <cstdint>

using namespace CLI;

struct drive_callbacks {
	std::function<void(const drive_create_options& options)> create;
	std::function<void(const drive_info_options& path)> info;
};

drive_create_options create_options;
drive_info_options info_options;
drive_callbacks callbacks;

void drive_create_setup(App* app);
void drive_info_setup(App* app);
void drive_create_set_bytes(const int& value);

void command_drive_setup(App* app) {
	drive_create_setup(app);
	drive_info_setup(app);
}

void command_drive_on_create(std::function<void(const drive_create_options& options)> callback)
{
	callbacks.create = callback;
}

void command_drive_on_info(std::function<void(const drive_info_options& path)> callback)
{
	callbacks.info = callback;
}

// === Internal ===

void drive_create_setup(App* parent)
{
	auto app = parent->add_subcommand("create", "Create new drive file");

	app->add_flag("-o,--overwrite", create_options.overwrite, "Overwrite disk file if exists");
	app->add_option("filename", create_options.path, "Path to disk file")
		->required()
		->check([](const std::string& value) {
			if (create_options.overwrite) return std::string();
			return NonexistentPath(value);
		});
	app->add_option("-s,--sector", create_options.sector, "Sector size (512,1024,...,4096)")
		->default_val(512)
		->check(Range(512, 4096))
		->check([](const std::string& value) {
			int int_value = std::stoi(value);
			if (int_value % 512 != 0) {
				return "Value must be a mupliple of 512";
			}
			return "";
		});
	app->add_option_function<int>("-b,--bytes", drive_create_set_bytes, "Disk size in bytes")
		->check(PositiveNumber)
		->check([](const std::string& value) {
			int int_value = std::stoi(value); 
			if (int_value % create_options.sector != 0) {
				return "Invalid disk size. It must be a multiple of " + std::to_string(create_options.sector);
			}
			return std::string();
		});
	app->add_option("--blocks", create_options.size, "Disk size in sectors")->excludes("-b")
		->check(PositiveNumber);
	
	app->callback([]() {
		if (create_options.size == 0) {
			throw ValidationError("Disk size cannot be zero");
		}
		callbacks.create(create_options);
	});
}

void drive_create_set_bytes(const int& value) {
	create_options.size = value / create_options.sector;
}

void drive_info_setup(App* parent) {
	auto app = parent->add_subcommand("info", "Get information about disk file");

	app->add_option("filename", info_options.path, "Path to disk file")
		->required()
		->check(ExistingFile);

	app->callback([]() {
		callbacks.info(info_options);
	});
}