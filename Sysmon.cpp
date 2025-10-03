#include <windows.h>
#include <psapi.h>
#include <tchar.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

// format bytes to human readable (MB/GB)
string prettySize(unsigned long long bytes) {
    const unsigned long long MB = 1024ULL * 1024ULL;
    const unsigned long long GB = MB * 1024ULL;
    stringstream ss;
    if (bytes >= GB) {
        double v = (double)bytes / (double)GB;
        ss.setf(std::ios::fixed); ss.precision(2);
        ss << v << " GB";
    }
    else {
        double v = (double)bytes / (double)MB;
        ss.setf(std::ios::fixed); ss.precision(1);
        ss << v << " MB";
    }
    return ss.str();
}

// --- CPU percent (system-wide) ---
// This is a simple snapshot-based method: compare two GetSystemTimes() samples.
double getCpuPercent() {
    static FILETIME prevIdle = { 0,0 };
    static FILETIME prevKernel = { 0,0 };
    static FILETIME prevUser = { 0,0 };

    FILETIME idleTime, kernelTime, userTime;
    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        return -1.0; // failed
    }

    // convert to 64-bit
    ULARGE_INTEGER idle, kernel, user;
    idle.LowPart = idleTime.dwLowDateTime;  idle.HighPart = idleTime.dwHighDateTime;
    kernel.LowPart = kernelTime.dwLowDateTime; kernel.HighPart = kernelTime.dwHighDateTime;
    user.LowPart = userTime.dwLowDateTime;   user.HighPart = userTime.dwHighDateTime;

    ULARGE_INTEGER prevIdleUI, prevKernelUI, prevUserUI;
    prevIdleUI.LowPart = prevIdle.dwLowDateTime;   prevIdleUI.HighPart = prevIdle.dwHighDateTime;
    prevKernelUI.LowPart = prevKernel.dwLowDateTime; prevKernelUI.HighPart = prevKernel.dwHighDateTime;
    prevUserUI.LowPart = prevUser.dwLowDateTime;   prevUserUI.HighPart = prevUser.dwHighDateTime;

    // first time just store values and return 0 (we don't have diff yet)
    static bool first = true;
    if (first) {
        prevIdle = idleTime;
        prevKernel = kernelTime;
        prevUser = userTime;
        first = false;
        return 0.0;
    }

    unsigned long long idleDiff = idle.QuadPart - prevIdleUI.QuadPart;
    unsigned long long kernelDiff = kernel.QuadPart - prevKernelUI.QuadPart;
    unsigned long long userDiff = user.QuadPart - prevUserUI.QuadPart;
    unsigned long long sysTotal = kernelDiff + userDiff;

    // save for next call
    prevIdle = idleTime;
    prevKernel = kernelTime;
    prevUser = userTime;

    if (sysTotal == 0) return 0.0;
    double busy = (double)(sysTotal - idleDiff);
    double percent = (busy * 100.0) / (double)sysTotal;
    return percent;
}

// --- Memory quick info ---
void printMemoryInfo() {
    MEMORYSTATUSEX m;
    m.dwLength = sizeof(m);
    if (GlobalMemoryStatusEx(&m)) {
        cout << "Memory: " << m.dwMemoryLoad << "% used  ("
            << prettySize(m.ullTotalPhys) << " total, "
            << prettySize(m.ullAvailPhys) << " free)" << endl;
    }
    else {
        cout << "Memory: (failed to read)" << endl;
    }
}

// --- Disk quick info (C: only, simple) ---
void printDiskInfo() {
    ULARGE_INTEGER freeAvail, totalBytes, freeBytes;
    if (GetDiskFreeSpaceEx(L"C:\\", &freeAvail, &totalBytes, &freeBytes)) {
        cout << "Disk C: " << prettySize(totalBytes.QuadPart) << " total, "
            << prettySize(freeBytes.QuadPart) << " free" << endl;
    }
    else {
        cout << "Disk C: (failed to read)" << endl;
    }
}

// --- Simple list of processes (PID and name) ---
void printProcessList() {
    DWORD pidArray[1024], bytesNeeded;
    if (!EnumProcesses(pidArray, sizeof(pidArray), &bytesNeeded)) {
        cout << "Processes: (failed to enumerate)" << endl;
        return;
    }

    unsigned int count = bytesNeeded / sizeof(DWORD);
    cout << "Processes (" << count << "):" << endl;

    for (unsigned int i = 0; i < count; ++i) {
        DWORD pid = pidArray[i];
        if (pid == 0) continue;

        TCHAR nameBuf[MAX_PATH] = TEXT("<unknown>");
        HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (h) {
            HMODULE mod;
            DWORD cbNeeded;
            if (EnumProcessModules(h, &mod, sizeof(mod), &cbNeeded)) {
                GetModuleBaseName(h, mod, nameBuf, sizeof(nameBuf) / sizeof(TCHAR));
            }
            CloseHandle(h);
        }
        // print in a simple human-friendly way
        wstring wname(nameBuf);
        string sname;
        // quick conversion wstring -> string (naive)
        for (wchar_t wc : wname) {
            if (wc < 128) sname.push_back((char)wc);
            else sname.push_back('?');
        }
        cout << "  PID " << pid << "  -  " << sname << endl;
    }
}

int main() {
    cout << "Sysmon Clone - starting... Press Ctrl+C to quit.\n\n";
    Sleep(800); // little pause so user sees startup text

    while (true) {
        system("cls"); // common student way to refresh console
        cout << "===== Sysmon CLONE C++ =====\n";

        double cpu = getCpuPercent();
        // show cpu nicely
        if (cpu < 0) cout << "CPU: (failed)\n";
        else {
            // print with 1 decimal
            cout.setf(std::ios::fixed); cout.precision(1);
            cout << "CPU: " << cpu << "%\n";
            cout.unsetf(std::ios::fixed);
        }

        printMemoryInfo();
        printDiskInfo();
        cout << "\n";
        printProcessList();

        cout << "\n(Refreshing every 3 seconds ...)\n";
        Sleep(3000);
    }

    return 0;
}
