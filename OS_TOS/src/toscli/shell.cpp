#include "shell.h"
#include "path.h"
#include "attr.h"

#include <iostream>
#include <fstream>
#include <filesystem>

using namespace fs;

std::vector<std::string> splitStringBySpace(const std::string& str);
void run(Filesystem& fs, Path& path, std::vector<std::string> parts);

std::map<std::string, int> parseFile(const std::filesystem::path& filename) {
	std::map<std::string, int> result;
	std::ifstream file(filename);

	if (!file.is_open()) {
		std::cerr << "Failed to open file: " << filename << std::endl;
		return result;
	}

	std::string line;
	while (std::getline(file, line)) {
		std::istringstream stream(line);
		std::string key;
		int value;

		if (stream >> key >> value) {
			result[key] = value;
		}
	}

	file.close();
	return result;
}


void shell(Filesystem& fs)
{
	Path path("/");
	do {
		std::string command;
		std::cout << path.get() << ">";
		std::getline(std::cin, command);

		if (command.empty()) continue;

		auto parts = splitStringBySpace(command);
		if (parts[0] == "exit") break;

		try {
			run(fs, path, parts);
		}
		catch (std::exception& e) {
			std::cout << e.what() << "\n";
		}
	} while (1);
}

void execute(fs::Filesystem& fs, std::string command, Path path)
{
	auto parts = splitStringBySpace(command);
	if (parts[0] == "exit") return;

	try {
		run(fs, path, parts);
	}
	catch (std::exception& e) {
		std::cout << e.what() << "\n";
	}
}

std::vector<std::string> splitStringBySpace(const std::string& str) {
	std::istringstream iss(str);
	std::vector<std::string> words;
	std::string word;
	while (iss >> word) {
		words.push_back(word);
	}
	return words;
}

void uploadDirectory(Filesystem& fs, const std::filesystem::path& dir, std::string path = "/") {
	static auto table = parseFile((dir / ".perms"));

	for (const auto& entry : std::filesystem::directory_iterator(dir)) {
		Path p1(path);
		p1.add_component(entry.path().filename().string());

		if (entry.path().filename() == ".perms") continue;

		if (std::filesystem::is_directory(entry.status())) {
			fs.create_catalog(p1);

			if (table.count(p1.get()) > 0) {
				auto vf = fs.open(p1);
				auto mode = table[p1.get()];
				auto fdata = fs.filedata(vf);

				auto attr = fs::FileAttr(fdata.fs_attr);
				uint8_t owner = (mode / 100) % 10;
				uint8_t group = (mode / 10) % 10;
				uint8_t world = mode % 10;
				attr.can(0, 0, owner & 1 << 2);
				attr.can(0, 1, owner & 1 << 1);
				attr.can(0, 2, owner & 1 << 0);
				attr.can(1, 0, group & 1 << 2);
				attr.can(1, 1, group & 1 << 1);
				attr.can(1, 2, group & 1 << 0);
				attr.can(2, 0, world & 1 << 2);
				attr.can(2, 1, world & 1 << 1);
				attr.can(2, 2, world & 1 << 0);
				fs.attr(vf, attr.value());

				fs.close(vf);
			}

			uploadDirectory(fs, entry.path(), path + entry.path().filename().string() + "/");
		}
		else if (std::filesystem::is_regular_file(entry.status())) {
			std::fstream f(entry.path(), std::ios::in | std::ios::binary);
			auto start = f.tellg();
			f.seekg(0, std::ios::end);
			auto buffer = std::vector<char>(f.tellg() - start);
			f.seekg(start);
			f.read(buffer.data(), buffer.size());
			f.close();

			if (fs.exists(p1)) fs.remove(p1);
			auto vf = fs.open(p1);
			fs.write(vf, buffer.data(), buffer.size());

			if (table.count(p1.get()) > 0) {
				auto mode = table[p1.get()];
				auto fdata = fs.filedata(vf);

				auto attr = fs::FileAttr(fdata.fs_attr);
				uint8_t owner = (mode / 100) % 10;
				uint8_t group = (mode / 10) % 10;
				uint8_t world = mode % 10;
				attr.can(0, 0, owner & 1 << 2);
				attr.can(0, 1, owner & 1 << 1);
				attr.can(0, 2, owner & 1 << 0);
				attr.can(1, 0, group & 1 << 2);
				attr.can(1, 1, group & 1 << 1);
				attr.can(1, 2, group & 1 << 0);
				attr.can(2, 0, world & 1 << 2);
				attr.can(2, 1, world & 1 << 1);
				attr.can(2, 2, world & 1 << 0);
				fs.attr(vf, attr.value());
			}

			fs.close(vf);
		}
	}
}

void run(Filesystem& fs, Path& path, std::vector<std::string> parts) {
	if (parts[0] == "ls") {
		auto files = fs.list_catalog(path);
		for (auto& f : files) {
			std::cout << f << "\n";
		}
	}
	else if (parts[0] == "mkdir") {
		Path p1(path);
		p1.add_component(parts[1]);
		fs.create_catalog(p1);
	}
	else if (parts[0] == "rmdir") {
		Path p1(path);
		p1.add_component(parts[1]);
		fs.remove(p1);
	}
	else if (parts[0] == "cat") {
		Path p1(path);
		p1.add_component(parts[1]);

		if (!fs.is_file(p1)) {
			throw std::runtime_error("File not found");
		}

		auto file = fs.open(p1);
		fs.seek(file, 0, FS_SEEK_END);
		auto size = fs.tell(file);
		fs.seek(file, 0, FS_SEEK_SET);

		auto buffer = (char*)malloc(size + 1);
		buffer[size] = '\0';
		fs.read(file, buffer, size);
		fs.close(file);

		if (size != 0) {
			std::cout << buffer << "\n";
		}
	}
	else if (parts[0] == "touch") {
		Path p1(path);
		p1.add_component(parts[1]);

		auto file = fs.open(p1);
		fs.write(file, "print('Hello World!')", 22);
		fs.close(file);
	}
	else if (parts[0] == "rm") {
		Path p1(path);
		p1.add_component(parts[1]);

		fs.remove(p1);
	}
	else if (parts[0] == "cd") {
		Path p1(path);
		if (parts[1] == "..") {
			p1.move_up();
		}
		else {
			p1.add_component(parts[1]);
		}

		if (!fs.is_catalog(p1)) {
			throw std::runtime_error("Catalog not found");
		}
		path = p1;
	}
	else if (parts[0] == "upload") {
		Path p1(path);
		p1.add_component(parts[2]);

		if (std::filesystem::is_directory(parts[1])) {
			uploadDirectory(fs, parts[1]);
		}
		else {
			std::fstream f(parts[1], std::ios::in | std::ios::binary);
			auto start = f.tellg();
			f.seekg(0, std::ios::end);
			auto buffer = std::vector<char>(f.tellg() - start);
			f.seekg(start);
			f.read(buffer.data(), buffer.size());
			f.close();

			if (fs.exists(p1)) fs.remove(p1);
			auto vf = fs.open(p1);
			fs.write(vf, buffer.data(), buffer.size());
			fs.close(vf);
		}		
	}
	else if (parts[0] == "download") {
		Path p1(path);
		p1.add_component(parts[1]);

		auto vf = fs.open(p1);
		fs.seek(vf, 0, FS_SEEK_END);
		auto size = fs.tell(vf);
		fs.seek(vf, 0);

		auto buffer = std::vector<char>(size, 0);
		fs.read(vf, buffer.data(), buffer.size());
		fs.close(vf);

		std::fstream f(parts[2], std::ios::out | std::ios::binary);
		f.write(buffer.data(), buffer.size());
		f.close();
	}
	else {
		std::cout << "Unknown command!\n";
	}
}
