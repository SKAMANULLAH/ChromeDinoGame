#include <windows.h>
#include <shlobj.h>
#include <string>

extern "C" const char _binary_ChromeDino_Windows_Portable_zip_start[];
extern "C" const char _binary_ChromeDino_Windows_Portable_zip_end[];

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    // 1. Directory: %LOCALAPPDATA%\ChromeDinoApp
    wchar_t localApp[MAX_PATH];
    if (FAILED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localApp))) {
        GetTempPathW(MAX_PATH, localApp);
    }
    std::wstring appDir = std::wstring(localApp) + L"\\ChromeDinoApp";
    CreateDirectoryW(appDir.c_str(), NULL);

    std::wstring exePath = appDir + L"\\ChromeDinoRaster.exe";
    std::wstring zipPath = appDir + L"\\payload.zip";

    // 2. Check if already extracted
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    bool needExtract = true;
    if (GetFileAttributesExW(exePath.c_str(), GetFileExInfoStandard, &fileInfo)) {
        if (fileInfo.nFileSizeLow > 100000) {
            needExtract = false;
        }
    }

    if (needExtract) {
        const char* pData = _binary_ChromeDino_Windows_Portable_zip_start;
        DWORD resSize = (DWORD)(_binary_ChromeDino_Windows_Portable_zip_end - _binary_ChromeDino_Windows_Portable_zip_start);

        if (pData && resSize > 0) {
            HANDLE hFile = CreateFileW(zipPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile != INVALID_HANDLE_VALUE) {
                DWORD written = 0;
                WriteFile(hFile, pData, resSize, &written, NULL);
                CloseHandle(hFile);

                // Unpack using Windows built-in tar.exe in < 0.5s
                std::wstring tarCmd = L"tar.exe -xf \"" + zipPath + L"\" -C \"" + appDir + L"\"";
                STARTUPINFOW si = { sizeof(si) };
                PROCESS_INFORMATION pi = { 0 };
                si.dwFlags = STARTF_USESHOWWINDOW;
                si.wShowWindow = SW_HIDE;
                if (CreateProcessW(NULL, &tarCmd[0], NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, appDir.c_str(), &si, &pi)) {
                    WaitForSingleObject(pi.hProcess, 30000);
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
                DeleteFileW(zipPath.c_str());
            }
        }
    }

    // 3. Launch ChromeDinoRaster.exe passing through any arguments
    LPCWSTR fullCmd = GetCommandLineW();
    LPCWSTR args = fullCmd;
    if (*args == L'"') {
        args++;
        while (*args && *args != L'"') args++;
        if (*args == L'"') args++;
    } else {
        while (*args && *args != L' ') args++;
    }
    while (*args == L' ') args++;

    std::wstring launchCmd = L"\"" + exePath + L"\" " + args;

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = nCmdShow;

    if (CreateProcessW(NULL, &launchCmd[0], NULL, NULL, FALSE, 0, NULL, appDir.c_str(), &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    return 0;
}
