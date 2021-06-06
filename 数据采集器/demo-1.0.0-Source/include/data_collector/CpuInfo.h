#ifndef CPU_INFO_H
#define CPU_INFO_H

#include <iostream>
#include <vector>
#include <fstream>
#include <numeric>
#ifndef __linux__
#include <conio.h>
#endif
#include "../../include/utils/Help.h"

class CpuInfo {
 public:
  float get_result();

 private:
#ifndef __linux__
  //HJS Method
  double FileTimeToDouble(FILETIME *pFiletime);
  double m_fOldCPUIdleTime;
  double m_fOldCPUKernelTime;
  double m_fOldCPUUserTime;
  bool Initialize();
  float GetCPUUseRate();

  uint64_t file_time_to_utc(const FILETIME *pfile_time);
  int get_processor_numbers(void);
  int get_cpu_usage(int pid);
  int get_process_pid(void);
#else
  size_t previous_idle_time = 0;
  size_t previous_total_time= 0;
  std::vector<size_t> get_cpu_times();
  bool get_cpu_times(size_t &idle_time, size_t &total_time);
#endif
};
#endif