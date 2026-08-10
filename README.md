# PC Hardware Inventory

A C++17 Windows console utility that collects and displays hardware and system inventory information using native Windows APIs.

## Features

- CPU model detection
- Logical processor count
- System architecture
- Windows product information
- Total installed RAM
- Fixed-drive inventory
- Disk capacity, free space and usage percentage
- Native Windows API / Registry access
- Clean console output

## Build

Build with MinGW g++:

```powershell
g++ -std=c++17 -Wall -Wextra -O2 -o pc-hardware-inventory.exe src/main.cpp -ladvapi32
```

Run:

```powershell
.\pc-hardware-inventory.exe
```

## Project Structure

```text
pc-hardware-inventory/
├── src/
│   └── main.cpp
└── README.md
```

## Roadmap

- GPU information through WMI
- SMBIOS motherboard information
- RAM module details
- Storage model/interface information
- Export to JSON
- Hardware health checks

## Technical Focus

The project focuses on native Windows system APIs and registry access rather than third-party hardware libraries.
