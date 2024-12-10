#pragma once

#include <string>
#include "tos.h"
#include "proc-executor.h"

typedef struct tos_config {
	std::string path;
} tos_config;

typedef struct tos_global {
	tos::TOS* os;
	tos::ProcessExecutor* executor;
} tos_global;

tos_global bootstrap(const tos_config& config);
void shutdown();