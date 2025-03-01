#include "system.h"

#include <unistd.h>

#include <cstddef>
#include <set>
#include <string>
#include <vector>
#include <algorithm>

#include "process.h"
#include "processor.h"
#include "linux_parser.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

// Done Return the system's CPU
Processor& System::Cpu() { return cpu_; }

// Optimized: Return a container composed of the system's processes
vector<Process>& System::Processes() {
    vector<int> pids = LinuxParser::Pids();  // Get current PIDs
    set<int> existingPids;
    
    // Make a copy of the current processes to check and update safely
    vector<Process> newProcesses;

    // Convert PIDs into Process objects (update existing processes if already present)
    for (int pid : pids) {
        existingPids.insert(pid);

        auto it = std::find_if(processes_.begin(), processes_.end(), [pid](const Process& process) {
            return process.Pid() == pid;
        });

        if (it == processes_.end()) {
            // Process not found, create new
            newProcesses.emplace_back(pid);
        } else {
            // Process exists, keep it in the new vector
            newProcesses.push_back(*it);
        }
    }

    // Remove processes that no longer exist
    processes_.clear();  // Clear the old vector
    for (const auto& pid : existingPids) {
        processes_.emplace_back(pid);  // Add only processes that are still alive
    }

    // Sort processes by CPU utilization (descending)
    std::sort(processes_.begin(), processes_.end(), [](const Process& a, const Process& b) {
        return a.CpuUtilization() > b.CpuUtilization();
    });

    return processes_;
}


// Done Return the system's kernel identifier (string)
std::string System::Kernel() { return LinuxParser::Kernel(); }

// Fixed: Return the system's memory utilization
float System::MemoryUtilization() {
    float memoryUtilization = LinuxParser::MemoryUtilization();
    return (memoryUtilization >= 0) ? memoryUtilization : 0.0f;  // Avoid invalid values
}

// Done Return the operating system name
std::string System::OperatingSystem() { return LinuxParser::OperatingSystem(); }

// Done Return the number of processes actively running on the system
int System::RunningProcesses() { return LinuxParser::RunningProcesses(); }

// Done Return the total number of processes on the system
int System::TotalProcesses() { return LinuxParser::TotalProcesses(); }

// Fixed: Ensure a valid system uptime
long int System::UpTime() { 
    long uptime = LinuxParser::UpTime();
    return uptime > 0 ? uptime : 1; // Ensure at least 1 second uptime to avoid division errors
}
