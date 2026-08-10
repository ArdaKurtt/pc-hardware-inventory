#define _WIN32_WINNT 0x0601
#include <windows.h>
#include <sysinfoapi.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>

#pragma comment(lib, "advapi32.lib")

namespace {

std::wstring registryString(HKEY root, const wchar_t* path, const wchar_t* value) {
    HKEY key{};
    if (RegOpenKeyExW(root, path, 0, KEY_READ | KEY_WOW64_64KEY, &key) != ERROR_SUCCESS)
        return L"";

    DWORD type = 0, size = 0;
    if (RegQueryValueExW(key, value, nullptr, &type, nullptr, &size) != ERROR_SUCCESS ||
        (type != REG_SZ && type != REG_EXPAND_SZ)) {
        RegCloseKey(key);
        return L"";
    }

    std::vector<wchar_t> buffer(size / sizeof(wchar_t) + 1);
    if (RegQueryValueExW(key, value, nullptr, &type,
                         reinterpret_cast<LPBYTE>(buffer.data()), &size) != ERROR_SUCCESS) {
        RegCloseKey(key);
        return L"";
    }

    RegCloseKey(key);
    return std::wstring(buffer.data());
}

std::wstring cpuName() {
    auto name = registryString(
        HKEY_LOCAL_MACHINE,
        L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        L"ProcessorNameString"
    );
    return name.empty() ? L"Unknown CPU" : name;
}

std::wstring windowsProduct() {
    auto name = registryString(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        L"ProductName"
    );
    return name.empty() ? L"Windows" : name;
}

void printMemory() {
    MEMORYSTATUSEX mem{};
    mem.dwLength = sizeof(mem);

    if (!GlobalMemoryStatusEx(&mem)) return;

    auto gb = [](ULONGLONG bytes) {
        return static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0);
    };

    std::wcout << L"Total RAM       : " << std::fixed << std::setprecision(1)
               << gb(mem.ullTotalPhys) << L" GB\n";
}

void printDisks() {
    std::wcout << L"\nFixed disks:\n";

    DWORD mask = GetLogicalDrives();
    for (wchar_t drive = L'A'; drive <= L'Z'; ++drive) {
        const int bit = drive - L'A';
        if (!(mask & (1u << bit))) continue;

        std::wstring root(1, drive);
        root += L":\\";

        if (GetDriveTypeW(root.c_str()) != DRIVE_FIXED) continue;

        ULARGE_INTEGER freeBytes{}, totalBytes{}, totalFree{};
        if (!GetDiskFreeSpaceExW(root.c_str(), &freeBytes, &totalBytes, &totalFree))
            continue;

        auto gb = [](ULONGLONG bytes) {
            return static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0);
        };

        const double used = 100.0 -
            static_cast<double>(totalFree.QuadPart) /
            static_cast<double>(totalBytes.QuadPart) * 100.0;

        std::wcout << L"  " << drive << L":  "
                   << std::fixed << std::setprecision(1)
                   << gb(totalBytes.QuadPart) << L" GB total, "
                   << gb(totalFree.QuadPart) << L" GB free, "
                   << used << L"% used\n";
    }
}

void printSystemInfo() {
    SYSTEM_INFO info{};
    GetSystemInfo(&info);

    std::wcout << L"===============================================\n";
    std::wcout << L"          PC HARDWARE INVENTORY v1.0\n";
    std::wcout << L"===============================================\n\n";

    std::wcout << L"Windows         : " << windowsProduct() << L"\n";
    std::wcout << L"CPU             : " << cpuName() << L"\n";
    std::wcout << L"Logical CPUs    : " << info.dwNumberOfProcessors << L"\n";
    std::wcout << L"Architecture    : ";

    if (info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
        std::wcout << L"x64";
    else if (info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM64)
        std::wcout << L"ARM64";
    else
        std::wcout << L"Other";

    std::wcout << L"\n";

    printMemory();
    printDisks();

    std::wcout << L"\nNotes:\n";
    std::wcout << L"- CPU model is read from Windows registry.\n";
    std::wcout << L"- RAM information comes from Windows memory APIs.\n";
    std::wcout << L"- Disk information is read from fixed logical drives.\n";
    std::wcout << L"- GPU/SMBIOS enumeration is planned for a future version.\n";
}

}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleTitleW(L"PC Hardware Inventory");
    printSystemInfo();

    std::wcout << L"\nPress Enter to close...";
    std::wstring line;
    std::getline(std::wcin, line);
    return 0;
}
