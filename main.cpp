#include <iostream>
#include <windows.h>
#include <tlhelp32.h>


bool IsProcessRunning(const wchar_t* processName, DWORD& processId) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &processEntry)) {
        do {
            if (_wcsicmp(processEntry.szExeFile, processName) == 0) {
                processId = processEntry.th32ProcessID;
                CloseHandle(snapshot);
                return true;
            }
        } while (Process32Next(snapshot, &processEntry));
    }

    CloseHandle(snapshot);
    return false;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Get current executable directory and run LOOT shortcut in MO2
    wchar_t buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::wstring programPath = buffer;
    size_t lastSlashPos = programPath.find_last_of(L"\\");
    programPath = programPath.substr(0, lastSlashPos);

    std::wstring moPath = programPath + L"\\..\\ModOrganizer.exe";
    std::wstring moArgument = L"moshortcut://LOOT";
    std::wstring commandLine = moPath + L" " + moArgument;

    wchar_t* commandLineArray = const_cast<wchar_t*>(commandLine.c_str());

    //Wait for the LOOT process to exit before passing it back to MO2
    if (CreateProcess(NULL, commandLineArray, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        Sleep(10000);

        const wchar_t* targetProcessName = L"LOOT.exe";
        DWORD processId;
        if (IsProcessRunning(targetProcessName, processId)) {
            HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, processId);
            if (hProcess != NULL) {
                WaitForSingleObject(hProcess, INFINITE);
                CloseHandle(hProcess);
            }
        }

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    return 0;
}
