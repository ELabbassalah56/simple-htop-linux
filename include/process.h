#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include "linux_parser.h"
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  Process(int pid);
  int Pid() const;                           
  std::string User();                      
  std::string Command();                   
  float CpuUtilization() const;                  
  std::string Ram();                       
  long int UpTime();
  void Update();                      
  bool operator<(Process const& a) const;
  std::pair<float, float> ParseCpuUtilization(const std::vector<std::string>& cpuUtilization) const; 
  
  private:
    int pid_;
    std::vector<std::string> cpuUtilization_ = {};
    long int upTime_;
    std::string user_;
    std::string command_;
    std::string ram_;
};

#endif