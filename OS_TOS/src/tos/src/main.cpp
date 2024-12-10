#include <iostream>
#include <string>

#include "bootstrap.h"
#include "ctrl_c.h"

int main(int argc, char** argv) {
	if (argc <= 1 || argc > 2) {
		std::cout << "Usage: tos [FILENAME]\n";
		return 0;
	}
	std::string path = argv[1];
	
	try {
		auto config = tos_config{ path };
		auto os = bootstrap(config);

		CtrlCLibrary::SetCtrlCHandler([os](CtrlCLibrary::CtrlSignal signal) {
			shutdown();
			return false;
		});

		if (os.os->fs().exists("/install.lua")) {
			auto pid = os.executor->spawn("/install.lua");
			os.executor->execute(pid);
		}

		auto pid2 = os.executor->spawn("/system/init.lua");
		os.executor->execute(pid2);

		shutdown();
	} catch (std::exception& e) {
		std::cerr << "Unable to run operating system: " << e.what();
	}
}