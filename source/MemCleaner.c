// Coded by DosX

#include <stdbool.h>
#include <stdio.h>
#include <windows.h>

#include "psapi.h"

#define true 0b1
#define false 0b0

#define GLOBAL_LOOP_DELAY 60000  // 60 seconds
#define PROCESSES_LOOP_DELAY 40  // 40 milliseconds

char *getFileNameFromPath(char *path) {  // Extract the file name from a given path
    for (size_t i = strlen(path) - 1; i; i--) {
        if (path[i] == '\\')
            return &path[i + 1];
    }
    return path;
}

int getFreeMemory() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(memInfo);

    GlobalMemoryStatusEx(&memInfo);

    // ULONGLONG totalPhysicalMemory = memInfo.ullTotalPhys;
    // ULONGLONG usedPhysicalMemory = totalPhysicalMemory - memInfo.ullAvailPhys;
    ULONGLONG freePhysicalMemory = memInfo.ullAvailPhys;

    return freePhysicalMemory / (1024 * 1024);
}

void main() {
    SetConsoleTitle("MemCleaner by DosX-dev (GitHub)");

    bool createdNew;

    // Create a mutex to ensure only one instance of the program runs
    HANDLE mutex = CreateMutex(NULL, TRUE, "memcleaner");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        exit(0x00);  // Exit if another instance is already running
    }

    while (true) {  // Continuous loop to clean memory and wait
        int freeMemoryBefore = getFreeMemory();

        DWORD processIds[1024], cbNeeded, cProcesses;
        unsigned int i;
        HANDLE hProcess;

        if (!EnumProcesses(processIds, sizeof(processIds), &cbNeeded)) {  // Enumerate all running processes
            return;
        }

        cProcesses = cbNeeded / sizeof(DWORD);
        for (i = 0; i < cProcesses; i++) {
            if (processIds[i] != 0) {
                hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processIds[i]);  // Open the process

                if (hProcess != NULL) {
                    if (hProcess) {
                        char pathbuf[MAX_PATH];

                        if (GetModuleFileNameEx(hProcess, NULL, pathbuf, MAX_PATH)) {
                            EmptyWorkingSet(hProcess);  // Clean the memory of a specific process
                            CloseHandle(hProcess);      // Close the handle

                            Sleep(PROCESSES_LOOP_DELAY);

                            printf("Memory cleaned for: %s [%i]\n", getFileNameFromPath(pathbuf), (int)hProcess);
                        }
                    }
                }
            }
        }
        printf("\nMemory cleaned: %d MB\nNext cycle in %d seconds.\n", getFreeMemory() - freeMemoryBefore, GLOBAL_LOOP_DELAY / 1000);

        Sleep(GLOBAL_LOOP_DELAY);
    }

    CloseHandle(mutex);
}
