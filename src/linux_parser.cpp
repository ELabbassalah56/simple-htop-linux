#include "linux_parser.h"

#include <unistd.h>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <pwd.h>
#include <sstream>
#include <string>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// Helper function to read a file into a string
static std::optional<string> ReadFile(const string& path) {
  std::ifstream file(path);
  if (!file.is_open()) return std::nullopt;
  std::ostringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

// Helper function to read a single line from a file
static std::optional<string> ReadLine(const string& path) {
  std::ifstream file(path);
  if (!file.is_open()) return std::nullopt;
  string line;
  std::getline(file, line);
  return line;
}

// Helper function to split a string by whitespace
static vector<string> SplitString(const string& str) {
  std::istringstream iss(str);
  return {std::istream_iterator<string>{iss}, std::istream_iterator<string>{}};
}

// Read and return the operating system name
string LinuxParser::OperatingSystem() {
  auto content = ReadFile(kOSPath);
  if (!content) return "Unknown OS";
  std::istringstream linestream(*content);
  string line, key, value;
  while (std::getline(linestream, line)) {
    std::istringstream iss(line);
    if (std::getline(iss, key, '=') && key == "PRETTY_NAME") {
      if (std::getline(iss >> std::ws, value)) {
        value.erase(std::remove(value.begin(), value.end(), '"'), value.end());
        return value;
      }
    }
  }
  return "Unknown OS";
}

// Read and return the kernel version
string LinuxParser::Kernel() {
  auto line = ReadLine(kProcDirectory + kVersionFilename);
  if (!line) return "Unknown Kernel";
  std::istringstream linestream(*line);
  string os, version, kernel;
  linestream >> os >> version >> kernel;
  return kernel;
}

// Read and return the list of process IDs
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  for (const auto& entry : std::filesystem::directory_iterator(kProcDirectory)) {
    if (entry.is_directory()) {
      string filename = entry.path().filename().string();
      if (std::all_of(filename.begin(), filename.end(), [](unsigned char c) { return std::isdigit(c); })) {
        try {
          pids.push_back(std::stoi(filename)); // Convert safely
        } catch (...) {
          continue; // Ignore any conversion errors
        }
      }
    }
  }
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  std::ifstream memInfoFile{kProcDirectory + kMeminfoFilename};
  if (!memInfoFile.is_open()) return 0.0f;

  std::string key;
  long memTotal = 0, memAvailable = 0, memFree = 0, buffers = 0, cached = 0;

  while (memInfoFile >> key) {
      if (key == "MemTotal:") memInfoFile >> memTotal;
      else if (key == "MemAvailable:") memInfoFile >> memAvailable;
      else if (key == "MemFree:") memInfoFile >> memFree;
      else if (key == "Buffers:") memInfoFile >> buffers;
      else if (key == "Cached:") memInfoFile >> cached;
  }
  memInfoFile.close();

  // If MemAvailable is missing, estimate it
  if (memAvailable == 0) memAvailable = memFree + buffers + cached;

  if (memTotal == 0) return 0.0f; // Prevent division by zero

  return (1.0f - (static_cast<float>(memAvailable) / memTotal)); // Convert to percentage
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  auto line = ReadLine(kProcDirectory + kUptimeFilename);
  if (!line) return 0;
  std::istringstream linestream(*line);
  long uptime;
  linestream >> uptime;
  return uptime;
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  auto line = ReadLine(kProcDirectory + kStatFilename);
  if (!line) return 0;
  auto values = SplitString(*line);
  if (values.size() < 8) return 0;
  return std::stol(values[1]) + std::stol(values[2]) + std::stol(values[3]) +
         std::stol(values[4]) + std::stol(values[5]) + std::stol(values[6]) +
         std::stol(values[7]);
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  auto line = ReadLine(kProcDirectory + to_string(pid) + kStatFilename);
  if (!line) return 0;
  auto values = SplitString(*line);
  if (values.size() < 17) return 0;
  return std::stol(values[13]) + std::stol(values[14]) + std::stol(values[15]) + std::stol(values[16]);
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  auto line = ReadLine(kProcDirectory + kStatFilename);
  if (!line) return 0;
  auto values = SplitString(*line);
  if (values.size() < 8) return 0;
  return std::stol(values[1]) + std::stol(values[2]) + std::stol(values[3]) +
         std::stol(values[6]) + std::stol(values[7]);
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  auto line = ReadLine(kProcDirectory + kStatFilename);
  if (!line) return 0;
  auto values = SplitString(*line);
  if (values.size() < 5) return 0;
  return std::stol(values[4]) + std::stol(values[5]);
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  auto line = ReadLine(kProcDirectory + kStatFilename);
  if (!line) return {};  // If file read fails, return an empty vector

  vector<string> cpuValues = SplitString(*line);

  if (cpuValues.empty() || cpuValues[0] != "cpu") {
      return {};  // Ensure first value is "cpu", otherwise return empty
  }

  cpuValues.erase(cpuValues.begin());  // Remove "cpu" label

  return cpuValues;  // Now contains only numeric CPU times
}


vector<string> LinuxParser::CpuUtilization(int pid) {
  auto line = ReadLine(kProcDirectory + to_string(pid) + kStatFilename);
  if (!line) return {"0", "0", "0", "0"};  // Ensure valid output if process is gone

  vector<string> cpuValues = SplitString(*line);

  if (cpuValues.size() < 17) return {"0", "0", "0", "0"};  // Ensure enough values exist

  // Return only fields 14–17 (utime, stime, cutime, cstime)
  return {cpuValues[13], cpuValues[14], cpuValues[15], cpuValues[16]};
}


// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  auto content = ReadFile(kProcDirectory + kStatFilename);
  if (!content) return -1;
  std::istringstream linestream(*content);
  string line, key;
  int value;
  while (std::getline(linestream, line)) {
    std::istringstream iss(line);
    iss >> key >> value;
    if (key == "processes") return value;
  }
  return -1;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  auto content = ReadFile(kProcDirectory + kStatFilename);
  if (!content) return -1;
  std::istringstream linestream(*content);
  string line, key;
  int value;
  while (std::getline(linestream, line)) {
    std::istringstream iss(line);
    iss >> key >> value;
    if (key == "procs_running") return value;
  }
  return -1;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  auto line = ReadLine(kProcDirectory + to_string(pid) + kCmdlineFilename);
  return (line && !line->empty()) ? *line : "N/A";  // Return "N/A" if file is empty
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  auto content = ReadFile(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (!content) return "N/A";  // Return "N/A" if the file couldn't be read
  
  std::istringstream linestream(*content);
  string line, key;
  long value;

  while (std::getline(linestream, line)) {
    std::istringstream iss(line);
    if (iss >> key >> value) {
      // Check if the key is VmRSS and process it
      if (key == "VmRSS:") {
        std::ostringstream oss;
        oss.precision(2);  // Keep two decimal places
        oss << std::fixed << (static_cast<double>(value) / 1024.0);  // Convert KB to MB
        return oss.str();
      }
    }
  }
  
  return "N/A";  // Return "N/A" if "VmRSS:" is not found
}

// Read and return the user ID associated with a process
std::optional<string> LinuxParser::Uid(int pid) {
  auto content = ReadFile(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (!content) return std::nullopt;  // If the content couldn't be read, return nullopt
  std::istringstream linestream(*content);
  string line, key, value;
  
  while (std::getline(linestream, line)) {
    std::istringstream iss(line);
    if (iss >> key >> value && key == "Uid:") {
      return value;  // Return the UID value found in the "Uid:" line
    }
  }
  return std::nullopt;  // If the "Uid:" key is not found
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  auto uidOpt = Uid(pid);
  if (!uidOpt) return "Unknown";  // If the UID couldn't be found, return "Unknown"

  try {
    int uid_num = std::stoi(uidOpt.value_or("0"));  // Ensure there's a fallback if value is empty
    struct passwd* pw = getpwuid(uid_num);
    return (pw != nullptr) ? pw->pw_name : "Unknown";  // Return the user name
  } catch (const std::exception&) {
    return "Unknown";  // Handle exceptions from std::stoi or getpwuid
  }
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  auto line = ReadLine(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (!line) return 0;
  auto values = SplitString(*line);
  if (values.size() <= 21) return 0;

  long start_time = std::stol(values[21]);
  long system_uptime = LinuxParser::UpTime();
  long clock_ticks = sysconf(_SC_CLK_TCK);
  
  return system_uptime - (start_time / clock_ticks); // Corrected order
}