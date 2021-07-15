#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "NtThings.h"

#include <iostream>
#include <tchar.h>
#include <list>
#include <vector>
#include <string>
#include <Psapi.h>
#include <VersionHelpers.h>
#include <Shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

using std::cout;
using std::cin;
using std::list;
using std::vector;

const int XOR_KEY{ 8 };

const std::vector<LPVOID> VC_PREF_BASES{ (void*)0x00000000DDDD1000,
                                       (void*)0x0000000010002000,
                                       (void*)0x0000000021003000,
                                       (void*)0x0000000032001000,
                                       (void*)0x0000000043002000,
                                       (void*)0x0000000050003000,
                                       (void*)0x0000000041001000,
                                       (void*)0x0000000042002000,
                                       (void*)0x0000000040003000,
                                       (void*)0x0000000022001000 };

// Helpers.cpp
LPVOID GetSuitableBaseAddress(HANDLE hProc, DWORD page_size, DWORD alloc_gran, DWORD vm_resv_c);
unsigned char* ReadProcessBlob(const char* sc_filename, DWORD* sc_size);
void EnableAnsiSupport();