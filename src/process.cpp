#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

#include "process.h"

using namespace LinuxParser;
using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) : pid_(pid) {
    Update();  // Fetch initial values
}

// Fetch updated process information
void Process::Update() {
    user_ = LinuxParser::User(pid_);
    command_ = LinuxParser::Command(pid_);
    cpuUtilization_ = LinuxParser::CpuUtilization(pid_);
    ram_ = LinuxParser::Ram(pid_);
    upTime_ = LinuxParser::UpTime(pid_);
}

// Return this process's ID
int Process::Pid() const { return pid_; }

// Optimized CPU Utilization
float Process::CpuUtilization() const {
    vector<string> cpuValues = LinuxParser::CpuUtilization(pid_);
    
    // Ensure that the vector has at least 4 elements for utime, stime, cutime, cstime
    if (cpuValues.size() < 4) return 0.0f;

    // Convert the CPU times to floats
    float utime = std::stof(cpuValues[0]);
    float stime = std::stof(cpuValues[1]);
    float cutime = std::stof(cpuValues[2]);
    float cstime = std::stof(cpuValues[3]);

    // Get system uptime and process start time
    long systemUpTime = LinuxParser::UpTime();

    // Ensure that the vector has at least 22 elements to access cpuValues[21]
    if (cpuValues.size() < 22) return 0.0f;

    long startTime = std::stol(cpuValues[21]) / sysconf(_SC_CLK_TCK);

    // Calculate process uptime
    long processUpTime = systemUpTime - startTime;

    // Avoid division by zero
    if (processUpTime <= 0) return 0.0f;

    // Calculate the total CPU time and return the utilization percentage
    float totalTime = utime + stime + cutime + cstime;
    return (totalTime / processUpTime); // Return percentage
}




// Safe Parsing of CPU Utilization
std::pair<float, float> Process::ParseCpuUtilization(const std::vector<std::string>& cpuUtilization) const {
    try {
        if (cpuUtilization.size() <= 17) return {0.0, 0.0};
        return {std::stof(cpuUtilization[13]), std::stof(cpuUtilization[14])};
    } catch (...) {
        return {0.0, 0.0};
    }
}

// Return the command that generated this process
string Process::Command() { return command_; }

// Return this process's memory utilization in MB
string Process::Ram() {
    try {
        int ramValue = std::stoi(ram_);
        return std::to_string(ramValue / 1024);
    } catch (...) {
        return "0";
    }
}

// Return the user that generated this process
string Process::User() { return user_; }

// Return the age of this process (in seconds)
long int Process::UpTime() { return upTime_; }

// Efficient Comparison Operator
bool Process::operator<(Process const& a) const { 
    return this->cpuUtilization_ > a.cpuUtilization_;
}
