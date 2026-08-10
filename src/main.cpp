#define _WIN32_WINNT 0x0601
#include <windows.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <sstream>

#pragma comment(lib, "iphlpapi.lib")

namespace {

std::string bytesToGB(ULONGLONG bytes) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(1)
        << (static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0)) << " GB";
    return out.str();
}

double cpuUsage(FILETIME idle1, FILETIME kernel1, FILETIME user1,
                FILETIME idle2, FILETIME kernel2, FILETIME user2) {
    ULARGE_INTEGER i1, k1, u1, i2, k2, u2;
    i1.LowPart = idle1.dwLowDateTime; i1.HighPart = idle1.dwHighDateTime;
    k1.LowPart = kernel1.dwLowDateTime; k1.HighPart = kernel1.dwHighDateTime;
    u1.LowPart = user1.dwLowDateTime; u1.HighPart = user1.dwHighDateTime;
    i2.LowPart = idle2.dwLowDateTime; i2.HighPart = idle2.dwHighDateTime;
    k2.LowPart = kernel2.dwLowDateTime; k2.HighPart = kernel2.dwHighDateTime;
    u2.LowPart = user2.dwLowDateTime; u2.HighPart = user2.dwHighDateTime;

    const ULONGLONG idle = i2.QuadPart - i1.QuadPart;
    const ULONGLONG kernel = k2.QuadPart - k1.QuadPart;
    const ULONGLONG user = u2.QuadPart - u1.QuadPart;
    const ULONGLONG total = kernel + user;

    if (total == 0) return 0.0;
    return 100.0 * static_cast<double>(total - idle) / static_cast<double>(total);
}

double cpuNow() {
    FILETIME idle1{}, kernel1{}, user1{};
    FILETIME idle2{}, kernel2{}, user2{};
    GetSystemTimes(&idle1, &kernel1, &user1);
    std::this_thread::sleep_for(std::chrono::milliseconds(350));
    GetSystemTimes(&idle2, &kernel2, &user2);
    return cpuUsage(idle1, kernel1, user1, idle2, kernel2, user2);
}

void showMemory() {
    MEMORYSTATUSEX mem{};
    mem.dwLength = sizeof(mem);
    if (!GlobalMemoryStatusEx(&mem)) return;

    const double used = 100.0 - static_cast<double>(mem.ullAvailPhys) /
                                  static_cast<double>(mem.ullTotalPhys) * 100.0;
    std::cout << "RAM Usage     : " << std::fixed << std::setprecision(1)
              << used << "%\n";
    std::cout << "RAM           : " << bytesToGB(mem.ullAvailPhys)
              << " free / " << bytesToGB(mem.ullTotalPhys) << " total\n";
}

void showDisk() {
    ULARGE_INTEGER freeBytes{}, totalBytes{}, totalFree{};
    if (!GetDiskFreeSpaceExW(L"C:\\", &freeBytes, &totalBytes, &totalFree)) return;

    const double used = 100.0 -
        static_cast<double>(totalFree.QuadPart) / static_cast<double>(totalBytes.QuadPart) * 100.0;

    std::cout << "Disk C:       : " << std::fixed << std::setprecision(1)
              << used << "% used\n";
    std::cout << "Disk Space    : " << bytesToGB(totalFree.QuadPart)
              << " free / " << bytesToGB(totalBytes.QuadPart) << " total\n";
}

bool pingLocal() {
    HANDLE handle = IcmpCreateFile();
    if (handle == INVALID_HANDLE_VALUE) return false;

    const char data[] = "PCMonitor";
    IPAddr target = inet_addr("1.1.1.1");
    char reply[sizeof(ICMP_ECHO_REPLY) + 64]{};

    DWORD result = IcmpSendEcho(
        handle, target, (LPVOID)data, sizeof(data),
        nullptr, reply, sizeof(reply), 1200
    );

    IcmpCloseHandle(handle);
    if (result == 0) return false;

    auto* response = reinterpret_cast<ICMP_ECHO_REPLY*>(reply);
    std::cout << "Latency       : " << response->RoundTripTime << " ms\n";
    return true;
}

void clearScreen() {
    system("cls");
}

void printHeader() {
    std::cout << "=============================================\n";
    std::cout << "        WINDOWS SYSTEM MONITOR v1.0\n";
    std::cout << "=============================================\n\n";
}

void render() {
    clearScreen();
    printHeader();

    std::cout << "CPU Usage     : " << std::fixed << std::setprecision(1)
              << cpuNow() << "%\n";
    showMemory();
    showDisk();
    std::cout << "Network       : ";
    if (!pingLocal()) std::cout << "Offline / test failed\n";

    std::cout << "\nRefresh rate  : 1 second\n";
    std::cout << "Press Ctrl+C to exit.\n";
}

}

int main() {
    SetConsoleTitleA("Windows System Monitor");
    while (true) {
        render();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
