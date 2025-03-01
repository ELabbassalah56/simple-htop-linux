/*
Key Files in /proc:
Process List:

Each running process has a directory under /proc/ named after its PID (e.g., /proc/1234/ for PID 1234).
To list all running processes, scan /proc/ for numeric directories.
Process Details:

/proc/[PID]/cmdline → Command-line arguments of the process.
/proc/[PID]/status → Human-readable process status.
/proc/[PID]/stat → Process details in a single line (raw format).
/proc/[PID]/exe → Symbolic link to the executable.
/proc/[PID]/fd/ → Open file descriptors.
System-Wide Process Information:

/proc/loadavg → System load information.
/proc/stat → Overall system statistics.
/proc/meminfo → Memory usage statistics.
/proc/uptime → System uptime.
*/
#ifndef SYSTEM_PARSER_H
#define SYSTEM_PARSER_H

#include <fstream>
#include <regex>
#include <string>
#include <optional>
#include <vector>

namespace LinuxParser {
// Paths
const std::string kProcDirectory{"/proc/"};
const std::string kCmdlineFilename{"/cmdline"};
const std::string kCpuinfoFilename{"/cpuinfo"};
const std::string kStatusFilename{"/status"};
const std::string kStatFilename{"/stat"};
const std::string kUptimeFilename{"/uptime"};
const std::string kMeminfoFilename{"/meminfo"};
const std::string kVersionFilename{"/version"};
const std::string kOSPath{"/etc/os-release"};
const std::string kPasswordPath{"/etc/passwd"};

// System
float MemoryUtilization();
long UpTime();
std::vector<int> Pids();
int TotalProcesses();
int RunningProcesses();
std::string OperatingSystem();
std::string Kernel();

// CPU
enum CPUStates {
  kUser_ = 0,
  kNice_,
  kSystem_,
  kIdle_,
  kIOwait_,
  kIRQ_,
  kSoftIRQ_,
  kSteal_,
  kGuest_,
  kGuestNice_
};
std::vector<std::string> CpuUtilization();
long Jiffies();
long ActiveJiffies();
long ActiveJiffies(int pid);
long IdleJiffies();

// Processes
std::string Command(int pid);
std::vector<std::string> CpuUtilization(int pid);
std::string Ram(int pid);
std::optional<std::string> Uid(int pid);
std::string User(int pid);
long int UpTime(int pid);
};  // namespace LinuxParser

#endif