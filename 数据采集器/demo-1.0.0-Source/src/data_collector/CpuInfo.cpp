#include "../../include/data_collector/CpuInfo.h"

/* HJS Method */
#ifndef __linux__
double CpuInfo::FileTimeToDouble(FILETIME *pFiletime) {
  return (double)((*pFiletime).dwHighDateTime * 4.294967296E9) +
         (double)(*pFiletime).dwLowDateTime;
}

bool CpuInfo::Initialize() {
  FILETIME ftIdle, ftKernel, ftUser;
  bool flag = false;
  flag = GetSystemTimes(&ftIdle, &ftKernel, &ftUser);
  if (!flag) {
    std::cout << "Error in GetSystemTimes!\n";
    return false;
  }
  m_fOldCPUIdleTime = FileTimeToDouble(&ftIdle);
  m_fOldCPUKernelTime = FileTimeToDouble(&ftKernel);
  m_fOldCPUUserTime = FileTimeToDouble(&ftUser);
  return flag;
}

float CpuInfo::GetCPUUseRate() {
  float nCPUUseRate = -1.0f;
  FILETIME ftIdle, ftKernel, ftUser;
  if (GetSystemTimes(&ftIdle, &ftKernel, &ftUser)) {
    double fCPUIdleTime = FileTimeToDouble(&ftIdle);
    double fCPUKernelTime = FileTimeToDouble(&ftKernel);
    double fCPUUserTime = FileTimeToDouble(&ftUser);
    nCPUUseRate = (float)(100.0 - (fCPUIdleTime - m_fOldCPUIdleTime) /
                                      (fCPUKernelTime - m_fOldCPUKernelTime +
                                       fCPUUserTime - m_fOldCPUUserTime) *
                                      100.0);
    m_fOldCPUIdleTime = fCPUIdleTime;
    m_fOldCPUKernelTime = fCPUKernelTime;
    m_fOldCPUUserTime = fCPUUserTime;
  }
  return nCPUUseRate;
}
#else
std::vector<size_t> CpuInfo::get_cpu_times() {
  std::ifstream proc_stat("/proc/stat");
  proc_stat.ignore(5, ' ');  // Skip the 'cpu' prefix.
  std::vector<size_t> times;
  for (size_t time; proc_stat >> time; times.push_back(time))
    ;
  return times;
}

bool CpuInfo::get_cpu_times(size_t &idle_time, size_t &total_time) {
  const std::vector<size_t> cpu_times = get_cpu_times();
  if (cpu_times.size() < 4) {
    return false;
  }
  idle_time = cpu_times[3];
  total_time = std::accumulate(cpu_times.begin(), cpu_times.end(), 0);
  return true;
}
#endif

float CpuInfo::get_result() {
  float cpu_usage = -1.0f;
#ifndef __linux__
  const int sleep_time = 1000;
  Sleep(sleep_time);  // ms
  cpu_usage = GetCPUUseRate();
#else
  int count = 0;
  for (size_t idle_time, total_time;
       get_cpu_times(idle_time, total_time) && (count <= 2);
       sleep(1), count++) {
    const float idle_time_delta = idle_time - previous_idle_time;
    const float total_time_delta = total_time - previous_total_time;
    cpu_usage = 100.0 * (1.0 - idle_time_delta / total_time_delta);
    previous_idle_time = idle_time;
    previous_total_time = total_time;
  }
#endif
  return cpu_usage;
}
