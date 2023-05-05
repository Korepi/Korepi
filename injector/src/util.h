#pragma once
#include <string>
#include <Windows.h>

#include <cheat-base/util.h>

int FindProcessId(const std::string& processName);
void WaitForCloseProcess(const std::string& processName);