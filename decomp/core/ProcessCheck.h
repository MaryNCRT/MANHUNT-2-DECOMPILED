// ProcessCheck.h — single-instance check used by WinMain.
#pragma once

#include <windows.h>

// 0x00615420 — number of running processes whose image name contains `name`.
unsigned CountProcessesNamed(const WCHAR* name);
