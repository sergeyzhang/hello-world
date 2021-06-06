#include "../../include/data_collector/ZxMethod.h"
#include <tlhelp32.h>

int ZxMethod::run_method() {
  int cpu_usage = -1;
  const int process_sleep_time = 1000;  // ms
  process_pid_ = get_process_pid();  // check: two devenv.exe may use same pid
  if (process_pid_ == 0) {
    std::cout << "Warning! get_process_pid() failed.\n";
    return -1;
  }
  for (int n = 0; n <= 2; n++) {
    cpu_usage = get_cpu_usage(process_pid_);
    Sleep(process_sleep_time);
  }
  return cpu_usage;
}

uint64_t ZxMethod::file_time_to_utc(const FILETIME *pfile_time) {
  LARGE_INTEGER large_integer;
  large_integer.LowPart = pfile_time->dwLowDateTime;
  large_integer.HighPart = pfile_time->dwHighDateTime;
  return large_integer.QuadPart;
}

int ZxMethod::get_processor_numbers() {
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  int processor_number = (int)info.dwNumberOfProcessors;
  return processor_number;
}

int ZxMethod::get_cpu_usage(int pid) {
  static int64_t last_time = 0;
  static int64_t last_system_time = 0;
  const int threshold = 0;
  int cpu_usage = -1;
  int processor_count = get_processor_numbers();
  GetSystemTimeAsFileTime(&now_);
  HANDLE open_process_succeed = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
  if (!open_process_succeed) {
    std::cout << "Error! OpenProcess() failed!\n";
    return -1;
  }
  bool get_process_times_succeed =
      GetProcessTimes(open_process_succeed, &creation_time_, &exit_time_,
                      &kernel_time_, &user_time_);
  if (!get_process_times_succeed) {
    std::cout << "Error! GetProcessTimes() failed!\n";
    return -1;
  }
  system_time_ =
      (file_time_to_utc(&kernel_time_) + file_time_to_utc(&user_time_)) /
      processor_count;
  time_ = file_time_to_utc(&now_);
  bool update_succeed = (last_system_time == 0) || (last_time == 0);
  if (update_succeed) {
    last_system_time = system_time_;
    last_time = time_;
    return 0;
  }
  system_time_delta_ = system_time_ - last_system_time;
  time_delta_ = time_ - last_time;
  if (time_delta_ == threshold) {
    std::cout << "Warning! CPU polls too fast!\n";
    return -1;
  }
  cpu_usage = (int)((system_time_delta_ * 100) / time_delta_);
  last_system_time = system_time_;
  last_time = time_;
  return cpu_usage;
}

int ZxMethod::get_process_pid() {
  std::string pid_name = "devenv.exe";
  DWORD dw_pid = 0;
  HANDLE process_handle;
  PROCESSENTRY32 process_entry_32;
  // Take a snapshot of all processes in the system.
  process_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (process_handle == INVALID_HANDLE_VALUE) {
    std::cout
        << "Error! CreateToolhelp32Snapshot() returns invalid handle value.\n";
    return 0;
  }
  process_entry_32.dwSize = sizeof(PROCESSENTRY32);
  if (!Process32First(process_handle, &process_entry_32)) {
    CloseHandle(process_handle);  // clean the snapshot object
    std::cout << "Error! Process32First() failed.\n";
    return 0;
  }
  // Find process_name in list and return its pid
  std::string ss;
  while (Process32Next(process_handle, &process_entry_32)) {
    for(int i = 0; i < sizeof(process_entry_32.szExeFile)/sizeof(process_entry_32.szExeFile[0]); i++){  //--
      ss += process_entry_32.szExeFile[i];
    }
    int ret = ss.compare(pid_name);
    if (ret == 0) {
      dw_pid = process_entry_32.th32ProcessID;
      break;
    }
  }
  CloseHandle(process_handle);
  return dw_pid;
}
