#ifndef METHOD_CPU_HJS_H
#define METHOD_CPU_HJS_H

#include <conio.h>

#include <stdio.h>

#include <iostream>

#include "../../include/utils/Help.h"

class HjsMethod {
 public:
  int run_method();

 private:
  double FileTimeToDouble(FILETIME *pFiletime);
  double m_fOldCPUIdleTime;
  double m_fOldCPUKernelTime;
  double m_fOldCPUUserTime;
  bool Initialize();
  int GetCPUUseRate();
};
#endif