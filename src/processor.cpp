#include "processor.h"
#include "linux_parser.h"
#include <vector>
#include <string>

float Processor::Utilization() {
    static long prevTotalTime = 0;
    static long prevActiveTime = 0;

    // Get latest CPU times from LinuxParser
    std::vector<std::string> cpuStrings = LinuxParser::CpuUtilization();

    if (cpuStrings.size() < 10) return 0.0f;  // Ensure valid data

    // Convert strings to long
    std::vector<long> cpuTimes;
    for (const auto& str : cpuStrings) {
        cpuTimes.push_back(std::stol(str));  // Convert each value to long
    }

    // Calculate Active and Total CPU time
    long user = cpuTimes[0];
    long nice = cpuTimes[1];
    long system = cpuTimes[2];
    long idle = cpuTimes[3];
    long iowait = cpuTimes[4];
    long irq = cpuTimes[5];
    long softirq = cpuTimes[6];
    long steal = cpuTimes[7];

    long activeTime = user + nice + system + irq + softirq + steal;
    long totalTime = activeTime + idle + iowait;

    // Compute deltas
    long totalDiff = totalTime - prevTotalTime;
    long activeDiff = activeTime - prevActiveTime;

    // Store previous values
    prevTotalTime = totalTime;
    prevActiveTime = activeTime;

    return (totalDiff > 0) ? static_cast<float>(activeDiff) / totalDiff : 0.0f;
}
