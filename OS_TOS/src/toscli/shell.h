#pragma once

#include "filesystem.h"

void shell(fs::Filesystem& fs); 
void execute(fs::Filesystem& fs, std::string command, fs::Path p = "/");