# Sysmon Clone (C++)

This is a small system monitoring tool that I made in **C++** for learning system programming and Windows API.  
It shows real-time stats like CPU usage, memory usage, disk space and also lists running processes.  
I wanted to build something like a mini version of **Sysmon / Task Manager** in the console.

🔹 Features
- Shows CPU usage in percentage  
- Displays memory load, total RAM and free RAM  
- Checks disk space (C: drive for now)  
- Lists all running processes with PID and name  
- Auto refresh every few seconds (like htop style)  


🔹 How It Works
- CPU usage is calculated with `GetSystemTimes`  
- Memory info comes from `GlobalMemoryStatusEx`  
- Disk space is read using `GetDiskFreeSpaceEx`  
- Processes are listed with `EnumProcesses` and `EnumProcessModules`  

🔹 How to Run
1. Clone the repo:
   ```bash
   git clone https://github.com/HexShieldX/sysmon-clone.git
   cd sysmon-clone
🔹 Sample Output
```text
CPU Usage: 25.9574%
Memory Usage: 78% (7998 MB Total, 1684 MB Free)
Disk C: 99 GB Total, 15 GB Free

Running Processes:
PID: 18140 - 000000940BEFF110
PID: 17004 - 000000940BEFF110
PID: 9284  - 000000940BEFF110
PID: 13556 - 000000940BEFF110
PID: 18428 - 000000940BEFF110
PID: 9756  - 000000940BEFF110
PID: 4624  - 000000940BEFF110






