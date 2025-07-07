# System Monitor

A real-time desktop system monitor built with C++ and Dear ImGui, providing comprehensive system information, process monitoring, and network statistics.

![System Monitor Screenshot](https://img.shields.io/badge/Platform-Linux-blue)
![Build Status](https://img.shields.io/badge/Build-Passing-green)
![Language](https://img.shields.io/badge/Language-C++-blue)

## Features

### 🖥️ System Information
- **Operating System**: Linux distribution detection
- **User Information**: Currently logged user
- **Hostname**: System hostname display
- **CPU Information**: Processor model and specifications
- **Task Count**: Real-time process count monitoring

### 📊 System Monitoring
- **CPU Usage**: Real-time CPU utilization with interactive graphs
- **Thermal Monitoring**: Temperature readings from system sensors
- **Fan Control**: Fan speed and status monitoring
- **Performance Graphs**: Customizable FPS and Y-scale controls
- **Animation Controls**: Start/stop graph animations

### 💾 Memory Management
- **RAM Usage**: Real-time memory utilization with visual progress bars
- **SWAP Usage**: Swap space monitoring and statistics
- **Disk Usage**: Storage space monitoring for root filesystem
- **Visual Indicators**: Progress bars with percentage and size information

### 🔍 Process Management
- **Process Table**: Comprehensive process list with sortable columns
- **Real-time Metrics**: PID, Name, State, CPU%, Memory%
- **Search Filter**: Filter processes by name or PID
- **Multi-selection**: Select multiple processes simultaneously
- **Process States**: Running, sleeping, zombie, stopped process detection

### 🌐 Network Monitoring
- **Interface Detection**: All network interfaces (lo, wlan, ethernet, docker)
- **IP Address Display**: IPv4 addresses for each interface
- **Traffic Statistics**: RX/TX bytes, packets, errors, drops
- **Visual Usage**: Network usage with automatic unit conversion (KB/MB/GB)
- **Detailed Tables**: Separate RX and TX statistics tables

## Screenshots

### Main Interface
The system monitor features a clean, tabbed interface with three main sections:

1. **System Window**: CPU, Fan, and Thermal monitoring tabs
2. **Memory & Processes Window**: RAM/SWAP usage and process table
3. **Network Window**: Interface information and traffic statistics

## Requirements

### System Requirements
- **OS**: Linux (Ubuntu 20.04+ recommended)
- **Architecture**: x86_64
- **Memory**: 512MB RAM minimum
- **Display**: OpenGL 3.0+ compatible graphics

### Dependencies
- **SDL2**: Graphics and window management
- **GLEW**: OpenGL extension loading
- **Dear ImGui**: Immediate mode GUI framework (included)
- **GCC**: C++11 compatible compiler

### Installation Commands
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install libsdl2-dev libglew-dev build-essential

# Fedora/RHEL
sudo dnf install SDL2-devel glew-devel gcc-c++

# Arch Linux
sudo pacman -S sdl2 glew gcc
```

## Building

### Quick Start
```bash
# Clone the repository
git clone <repository-url>
cd system-monitor

# Install dependencies (Ubuntu/Debian)
sudo apt update && sudo apt install libsdl2-dev libglew-dev build-essential

# Build the application
make clean && make

# Run the system monitor
./monitor
```

### Note
The `monitor` executable and build artifacts (`*.o` files) are automatically excluded from git via `.gitignore`. After cloning, simply run `make` to build the application.

### Build Process
The project uses a Makefile with the following targets:
- `make` or `make all`: Build the application
- `make clean`: Remove build artifacts
- `make rebuild`: Clean and build

### Build Output
- **Executable**: `monitor`
- **Object Files**: `*.o` (automatically cleaned)

## Usage

### Running the Application
```bash
./monitor
```

### Interface Navigation
- **System Tab**: Switch between CPU, Fan, and Thermal monitoring
- **Memory Section**: View RAM, SWAP, and disk usage
- **Process Filter**: Type to search processes by name or PID
- **Network Interfaces**: Click tabs to view RX/TX statistics

### Controls
- **FPS Slider**: Adjust graph update rate (1-120 FPS)
- **Y-Scale Slider**: Modify graph vertical scale
- **Animation Toggle**: Pause/resume graph animations
- **Process Selection**: Click processes to select/deselect

### Keyboard Shortcuts
- **Ctrl+C**: Exit application (shows debug statistics)
- **Tab**: Navigate between UI elements
- **Enter**: Confirm selections

## Technical Details

### Architecture
- **Frontend**: Dear ImGui with OpenGL 3.0 backend
- **Backend**: Direct Linux system file reading (`/proc`, `/sys`)
- **Graphics**: SDL2 for window management and input
- **Data Sources**: `/proc/meminfo`, `/proc/stat`, `/proc/net/dev`, `/proc/cpuinfo`

### Performance
- **Update Frequency**: 
  - System metrics: 60 FPS (configurable)
  - Process list: 1 second intervals
  - Network stats: Real-time
- **Memory Usage**: ~100MB typical
- **CPU Impact**: <1% on modern systems

### Data Accuracy
All displayed values are verified against standard Linux commands:
- **Memory**: Matches `free -h` output
- **Processes**: Matches `top` and `ps` commands
- **Network**: Matches `ifconfig` and `/proc/net/dev`
- **System Info**: Matches `uname`, `hostname`, `who` commands

## Development

### Project Structure
```
system-monitor/
├── main.cpp           # Application entry point and main loop
├── system.cpp         # System information and monitoring
├── mem.cpp           # Memory and process management
├── network.cpp       # Network interface monitoring
├── header.h          # Common headers and declarations
├── Makefile          # Build configuration
├── imgui/            # Dear ImGui library
└── docs/             # Documentation and reports
```

### Adding Features
1. **New Metrics**: Add data collection in appropriate `.cpp` file
2. **UI Components**: Extend render functions with ImGui widgets
3. **System Integration**: Read from `/proc` or `/sys` filesystems

### Testing
The project includes comprehensive audit testing:
```bash
# Run automated tests
python3 test_audit.py

# Verify system values
./verify_system_values.sh

# Check build and functionality
make && timeout 5s ./monitor
```

## Troubleshooting

### Common Issues

**Build Errors**
```bash
# Missing dependencies
sudo apt install libsdl2-dev libglew-dev

# Permission issues
chmod +x monitor
```

**Runtime Issues**
```bash
# Missing libraries
ldd ./monitor  # Check library dependencies

# Permission denied for /proc files
# Run as regular user (not root)
```

**Display Issues**
```bash
# OpenGL compatibility
glxinfo | grep "OpenGL version"  # Check OpenGL support
```

### Debug Information
The application provides debug statistics on exit (Ctrl+C):
- Memory calculations verification
- Process count validation
- System metric accuracy checks

## Contributing

### Development Setup
1. Fork the repository
2. Install dependencies
3. Build and test locally
4. Submit pull requests

### Code Style
- **C++11 Standard**: Modern C++ features
- **ImGui Patterns**: Immediate mode GUI best practices
- **Linux Integration**: Direct system file reading
- **Error Handling**: Graceful fallbacks for missing data

## License

This project is open source. See LICENSE file for details.

## Acknowledgments

- **Dear ImGui**: Excellent immediate mode GUI framework
- **SDL2**: Cross-platform graphics and input
- **Linux Community**: Comprehensive `/proc` filesystem documentation

---

**System Monitor** - Real-time Linux system monitoring made simple.
