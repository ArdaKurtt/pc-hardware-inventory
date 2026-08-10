# Windows System Monitor

A C++17 Windows console application for real-time monitoring of CPU, RAM, disk and network performance.

## Features

- Real-time CPU usage measurement using `GetSystemTimes`
- RAM usage and available/total memory
- C: drive usage and free space
- ICMP latency test
- Automatic screen refresh
- Lightweight Windows API implementation
- No external runtime dependencies

## Build

Build with MinGW g++ on Windows:

```powershell
g++ -std=c++17 -Wall -Wextra -O2 -o windows-system-monitor.exe src/main.cpp -liphlpapi -lws2_32
```

Run:

```powershell
.\windows-system-monitor.exe
```

Press `Ctrl+C` to exit.

## Project Structure

```text
windows-system-monitor/
├── src/
│   └── main.cpp
└── README.md
```

## Technical Focus

This project demonstrates Windows API usage, performance measurement, system resource monitoring, ICMP networking and a continuously refreshing console interface.
