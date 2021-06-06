#include "../../include/data_collector/HjsMethod.h"

int HjsMethod::run_method() {
  if (!Initialize()) {
    std::cout << "INIT FAIL\n";
    return -1;
  };
  const int sleep_time = 1000;
  Sleep(sleep_time);  // ms
  int cpu_used = GetCPUUseRate();
  return cpu_used;
}

double HjsMethod::FileTimeToDouble(FILETIME *pFiletime) {
  return (double)((*pFiletime).dwHighDateTime * 4.294967296E9) +
         (double)(*pFiletime).dwLowDateTime;
}

bool HjsMethod::Initialize() {
  FILETIME ftIdle, ftKernel, ftUser;
  bool flag = FALSE;
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

int HjsMethod::GetCPUUseRate() {
  int nCPUUseRate = -1;
  FILETIME ftIdle, ftKernel, ftUser;
  if (GetSystemTimes(&ftIdle, &ftKernel, &ftUser)) {
    double fCPUIdleTime = FileTimeToDouble(&ftIdle);
    double fCPUKernelTime = FileTimeToDouble(&ftKernel);
    double fCPUUserTime = FileTimeToDouble(&ftUser);
    nCPUUseRate = (int)(100.0 - (fCPUIdleTime - m_fOldCPUIdleTime) /
                                    (fCPUKernelTime - m_fOldCPUKernelTime +
                                     fCPUUserTime - m_fOldCPUUserTime) *
                                    100.0);
    m_fOldCPUIdleTime = fCPUIdleTime;
    m_fOldCPUKernelTime = fCPUKernelTime;
    m_fOldCPUUserTime = fCPUUserTime;
  }
  return nCPUUseRate;
}