#ifndef METHOD_CPU_ZX_H
#define METHOD_CPU_ZX_H

#include <stdio.h>

#include <iostream>

#include "../../include/utils/Help.h"

typedef long long int64_t;
typedef unsigned long long uint64_t;

using namespace std;

class ZxMethod {
 public:
  int run_method(void);

 private:
  uint64_t file_time_to_utc(const FILETIME *pfile_time);
  int get_processor_numbers(void);
  int get_cpu_usage(int pid);
  int get_process_pid(void);

  FILETIME now_;
  FILETIME creation_time_;
  FILETIME exit_time_;
  FILETIME kernel_time_;
  FILETIME user_time_;
  int64_t system_time_;
  int64_t time_;
  int64_t system_time_delta_;
  int64_t time_delta_;
  int process_pid_;
};
#endif